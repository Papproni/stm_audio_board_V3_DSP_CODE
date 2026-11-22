"""SABV3 application firmware updater (USB CDC)."""
import struct
import threading
import time
import zlib
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

import serial
from serial.tools import list_ports

MAGIC = 0x55424153  # b"SABU" in little-endian memory
CMD_HELLO, CMD_ERASE, CMD_WRITE, CMD_RESET, CMD_VERIFY = 1, 2, 3, 4, 5
CHUNK = 256  # Must be a multiple of the STM32H723 32-byte flash word.
APP_AREA_SIZE = 0x080E0000 - 0x08020000  # Never erase the preset sector at 0x080E0000.
APP_HELLO_SIZE = 0x08100000 - 0x08020000


class ApplicationDetected(RuntimeError):
    """The selected CDC port is the running DSP application, not its bootloader."""


class Updater:
    def __init__(self, port: str, log, progress):
        self.port, self.log, self.progress, self.seq = port, log, progress, 1
        self.bootloader_verified = False
        self.serial = serial.Serial(port, 115200, timeout=1, write_timeout=3)
        time.sleep(0.15)

    def _attempt_reconnect(self, timeout_s: float):
        """Reopen only a CDC port that proves it is the DSP bootloader."""
        deadline = time.monotonic() + max(0.0, timeout_s or 0)
        self.bootloader_verified = False
        try:
            self.serial.close()
        except Exception:
            pass

        while time.monotonic() < deadline:
            for p in list_ports.comports():
                if (getattr(p, "vid", None), getattr(p, "pid", None)) != (0x0483, 0x2FBE):
                    continue
                candidate = None
                try:
                    candidate = serial.Serial(p.device, 115200, timeout=0.2, write_timeout=3)
                    self.serial = candidate
                    self.seq = 1
                    if self.command(CMD_HELLO, timeout=1.0) == APP_AREA_SIZE:
                        self.port = p.device
                        self.bootloader_verified = True
                        self.log(f"Verified DSP bootloader on {self.port}")
                        return
                except (serial.SerialException, OSError, RuntimeError):
                    pass
                if candidate is not None:
                    try:
                        candidate.close()
                    except (serial.SerialException, OSError):
                        pass
            time.sleep(0.2)
        raise RuntimeError("DSP bootloader did not re-enumerate or failed HELLO")

    def close(self):
        self.serial.close()

    def packet(self, command: int, payload=b""):
        header = struct.pack("<IBBHI", MAGIC, command, 0, len(payload), self.seq)
        result = header + payload + struct.pack("<I", zlib.crc32(header + payload) & 0xFFFFFFFF)
        sequence = self.seq
        self.seq += 1
        return result, sequence

    def command(self, command: int, payload=b"", timeout=25, reply=True):
        packet, sequence = self.packet(command, payload)
        self.serial.write(packet)
        self.serial.flush()
        if(reply == False):
            return
        deadline = time.monotonic() + timeout  # Sector erase may take several seconds.
        received = bytearray()
        while time.monotonic() < deadline:
            received.extend(self.serial.read(16 - len(received)))
            if len(received) < 16:
                continue
            magic, reply, status, _, reply_seq, detail = struct.unpack("<IBBHII", received[:16])
            if magic == MAGIC and reply == (command | 0x80) and reply_seq == sequence:
                if status:
                    raise RuntimeError(f"Device rejected command {command} (error {status})")
                return detail
            received.clear()
        raise RuntimeError("Timed out waiting for device response")
    
    def wait_for_bootloader(self):
        # First try the already-open port (device may have booted directly into BL).
        identity = next(((p.vid, p.pid) for p in list_ports.comports()
                         if p.device == self.port), None)
        if identity not in ((0x0483, 0x2FC4), (0x0483, 0x2FBE)):
            raise RuntimeError("Selected port is not the DSP application or bootloader")
        for _ in range(15):
            try:
                size = self.command(CMD_HELLO, timeout=1.5)
                if size == APP_HELLO_SIZE:
                    if identity != (0x0483, 0x2FC4):
                        raise RuntimeError("Unexpected SABU identity on bootloader port")
                    raise ApplicationDetected("Running application; requesting bootloader")
                if size != APP_AREA_SIZE or identity != (0x0483, 0x2FBE):
                    raise RuntimeError(f"Not the DSP bootloader (app area {size // 1024} KiB)")
                self.bootloader_verified = True
                self.log(f"Bootloader ready; app area: {size // 1024} KiB")
                return
            except ApplicationDetected:
                raise
            except (RuntimeError, serial.SerialException):
                time.sleep(0.3)
        raise RuntimeError("SAB bootloader was not found on the selected COM port")

    def enter_from_app(self):
        reset_frame, _ = self.packet(CMD_RESET)
        self.serial.write(reset_frame)
        self.serial.flush()
        self.serial.close()
        self.bootloader_verified = False

    @staticmethod
    def _application_hello(port):
        """Confirm the restarted CDC port speaks the running DSP's SABQ protocol."""
        request = b"SABQ\x01\x01" + struct.pack("<HH", 1, 0)
        request += struct.pack("<I", zlib.crc32(request) & 0xFFFFFFFF)
        with serial.Serial(port, 115200, timeout=0.5, write_timeout=2) as device:
            device.reset_input_buffer()
            device.write(request)
            header = device.read(10)
            if len(header) != 10 or header[:6] != b"SABQ\x81\x01":
                return False
            sequence, length = struct.unpack_from("<HH", header, 6)
            if sequence != 1 or not 5 <= length <= 512:
                return False
            body = device.read(length + 4)
            if len(body) != length + 4 or body[0] != 0:
                return False
            crc = struct.unpack_from("<I", body, length)[0]
            return (crc == zlib.crc32(header + body[:length]) & 0xFFFFFFFF and
                    struct.unpack_from("<I", body, 1)[0] == 0x51424153)

    def wait_for_application(self, timeout_s=20):
        try:
            self.serial.close()
        except (serial.SerialException, OSError):
            pass
        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            for port in list_ports.comports():
                if (port.vid, port.pid) != (0x0483, 0x2FC4):
                    continue
                try:
                    if self._application_hello(port.device):
                        self.port = port.device
                        self.log(f"Updated application answered SABQ HELLO on {self.port}")
                        return
                except (serial.SerialException, OSError):
                    pass
            time.sleep(0.3)
        raise RuntimeError("Flash verified, but updated application did not answer USB HELLO")

    def upload(self, firmware: Path):
        if not getattr(self, "bootloader_verified", False):
            raise RuntimeError("Upload blocked: bootloader identity not verified")
        image = firmware.read_bytes()
        if len(image) < 8:
            raise RuntimeError("Selected file is not an application binary")
        if len(image) > APP_AREA_SIZE:
            raise RuntimeError("Firmware exceeds the DSP application area (preset sector is protected)")
        stack, reset = struct.unpack_from("<II", image)
        if not (0x24000000 <= stack <= 0x24080000 and stack % 8 == 0 and
                reset & 1 and 0x08020000 <= (reset & ~1) < 0x08020000 + len(image)):
            raise RuntimeError("Invalid DSP application vector table; nothing was erased")
        image += b"\xFF" * ((-len(image)) % 32)
        self.progress(0, len(image))
        sectors = (len(image) + 0x1FFFF) // 0x20000
        self.log(f"Erasing {sectors} application sector(s) for {len(image):,} bytes…")
        for sector in range(1, sectors + 1):
            self.command(CMD_ERASE, bytes([sector]), timeout=15)
        self.log(f"Erased sectors 1–{sectors}; preset sector 7 untouched")
        for offset in range(0, len(image), CHUNK):
            self.command(CMD_WRITE, struct.pack("<I", offset) + image[offset:offset + CHUNK])
            written = min(offset + CHUNK, len(image))
            self.progress(written, len(image))
            self.log(f"Written {written:,}/{len(image):,} bytes ({written * 100 // len(image)}%)")
        device_crc = self.command(CMD_VERIFY, struct.pack("<II", 0, len(image)), timeout=20)
        expected_crc = zlib.crc32(image) & 0xFFFFFFFF
        if device_crc != expected_crc:
            raise RuntimeError(f"Flash CRC mismatch: expected {expected_crc:08X}, got {device_crc:08X}")
        self.log("Bootloader verified application flash CRC")
        self.command(CMD_RESET, reply=False)
        self.wait_for_application()
        self.log("Update complete; application restarted")


class Window(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("SABV3 Firmware Updater")
        self.resizable(False, False)
        self.port = tk.StringVar()
        self.firmware = tk.StringVar()
        root = ttk.Frame(self, padding=12)
        root.grid()
        ttk.Label(root, text="USB serial port").grid(row=0, column=0, sticky="w")
        self.ports = ttk.Combobox(root, textvariable=self.port, width=42, state="readonly")
        self.ports.grid(row=0, column=1, padx=6)
        ttk.Button(root, text="Refresh", command=self.refresh).grid(row=0, column=2)
        ttk.Label(root, text="Application .bin").grid(row=1, column=0, sticky="w", pady=(8, 0))
        ttk.Entry(root, textvariable=self.firmware, width=45).grid(row=1, column=1, padx=6, pady=(8, 0))
        ttk.Button(root, text="Choose…", command=self.choose).grid(row=1, column=2, pady=(8, 0))
        self.go = ttk.Button(root, text="Enter update mode and upload", command=self.start)
        self.go.grid(row=2, column=0, columnspan=3, sticky="ew", pady=12)
        self.progress_text = tk.StringVar(value="Ready")
        ttk.Label(root, textvariable=self.progress_text).grid(row=3, column=0, columnspan=3, sticky="w")
        self.progress = ttk.Progressbar(root, length=500, mode="determinate", maximum=100)
        self.progress.grid(row=4, column=0, columnspan=3, sticky="ew", pady=(2, 8))
        self.bootloader_text = tk.StringVar(value="SAB bootloader: none")
        ttk.Label(root, textvariable=self.bootloader_text).grid(row=5, column=0, columnspan=3, sticky="w")
        self.output = tk.Text(root, width=66, height=10, state="disabled")
        self.output.grid(row=6, column=0, columnspan=3)
        self.bootloader_ports = set()
        self.last_port_details = {}
        self.refresh()
        self.scan_for_bootloader()

    def refresh(self):
        available = list(list_ports.comports())
        values = [p.device for p in available]
        self.ports["values"] = values
        if values and self.port.get() not in values:
            dsp = next((p.device for p in available
                        if (p.vid, p.pid) in ((0x0483, 0x2FC4), (0x0483, 0x2FBE))), None)
            self.port.set(dsp or values[0])

    def scan_for_bootloader(self):
        current_ports = list_ports.comports()
        new_bootloader_ports = set()
        port_details = []

        current_devices = set()

        for port in current_ports:
            # pyserial's ListPortInfo has .vid and .pid when available
            vid = getattr(port, "vid", None)
            pid = getattr(port, "pid", None)
            manufacturer = port.manufacturer or ""
            product = port.product or ""
            description = port.description or ""

            if (vid, pid) == (0x0483, 0x2FBE):
                new_bootloader_ports.add(port.device)

            # Always prepare the details string and only log if it changed.
            details = f"{port.device}: vid={vid}, pid={pid}, manufacturer='{manufacturer}', product='{product}'"
            port_details.append(details)
            current_devices.add(port.device)
            if self.last_port_details.get(port.device) != details:
                # Only log changed/new details
                self.log(details)
                self.last_port_details[port.device] = details

        if new_bootloader_ports != self.bootloader_ports:
            self.bootloader_ports = new_bootloader_ports
            if self.bootloader_ports:
                ports_list = ", ".join(sorted(self.bootloader_ports))
                self.bootloader_text.set(f"SAB bootloader: {ports_list}")
                self.log(f"SAB_bootloader detected on {ports_list}")
                # Auto-select the first detected bootloader port
                try:
                    candidate = sorted(self.bootloader_ports)[0]
                    # Ensure combobox includes the candidate
                    values = list(self.ports["values"]) if self.ports["values"] else []
                    if candidate not in values:
                        # refresh the list from the system ports
                        values = [p.device for p in list_ports.comports()]
                        self.ports["values"] = values
                    self.port.set(candidate)
                    self.log(f"Auto-selected port {candidate}")
                except Exception:
                    pass
            else:
                self.bootloader_text.set("SAB bootloader: none")
                self.log("SAB_bootloader no longer detected")

        # Remove details for disappeared ports so they can be logged again if reconnected
        removed = set(self.last_port_details) - current_devices
        for dev in removed:
            del self.last_port_details[dev]

        self.after(500, self.scan_for_bootloader)

    def choose(self):
        name = filedialog.askopenfilename(filetypes=[("Firmware binary", "*.bin"), ("All files", "*.*")])
        if name:
            self.firmware.set(name)

    def log(self, text):
        self.output.configure(state="normal")
        self.output.insert("end", text + "\n")
        self.output.see("end")
        self.output.configure(state="disabled")

    def set_progress(self, written, total):
        percent = 0 if total == 0 else written * 100 / total
        self.progress["value"] = percent
        self.progress_text.set(f"Transfer: {written:,} / {total:,} bytes ({percent:.1f}%)")

    def start(self):
        if not self.port.get() or not self.firmware.get():
            messagebox.showerror("Missing selection", "Choose both a COM port and an application .bin file.")
            return
        self.go.configure(state="disabled")
        self.set_progress(0, 0)
        threading.Thread(target=self.worker, args=(self.port.get(), self.firmware.get()), daemon=True).start()

    def worker(self, port, firmware):
        updater = None
        try:
            path = Path(firmware)
            progress = lambda done, total: self.after(0, self.set_progress, done, total)
            updater = Updater(port, lambda text: self.after(0, self.log, text), progress)
            try:
                updater.wait_for_bootloader()
            except ApplicationDetected:
                self.after(0, self.log, "Requesting update mode from running application…")
                updater.enter_from_app()
                updater._attempt_reconnect(20)
            updater.upload(path)
            self.after(0, messagebox.showinfo, "SAB updater", "Firmware update completed.")
        except Exception as exc:
            self.after(0, messagebox.showerror, "SAB updater", str(exc))
        finally:
            if updater:
                updater.close()
            self.after(0, self.go.configure, {"state": "normal"})


if __name__ == "__main__":
    Window().mainloop()
