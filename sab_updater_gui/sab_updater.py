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
CMD_HELLO, CMD_ERASE, CMD_WRITE, CMD_RESET = 1, 2, 3, 4
CHUNK = 256  # Must be a multiple of the STM32H723 32-byte flash word.


class Updater:
    def __init__(self, port: str, log, progress):
        self.port, self.log, self.progress, self.seq = port, log, progress, 1
        self.serial = serial.Serial(port, 115200, timeout=1, write_timeout=3)
        time.sleep(0.15)

    def _attempt_reconnect(self, timeout_s: float):
        """Try to reopen the serial port within timeout_s seconds.
        Prefer the same port name, otherwise search for the known VID/PID.
        On success replace self.serial and reset sequence counter.
        """
        deadline = time.monotonic() + max(0.0, timeout_s or 0)
        try:
            self.serial.close()
        except Exception:
            pass

        while time.monotonic() < deadline:
            # First try same port name
            try:
                self.serial = serial.Serial(self.port, 115200, timeout=1, write_timeout=3)
                time.sleep(0.15)
                self.seq = 1
                return
            except (serial.SerialException, OSError):
                # search for device by VID/PID
                for p in list_ports.comports():
                    if getattr(p, "vid", None) == 1155 and getattr(p, "pid", None) == 12222:
                        try:
                            self.port = p.device
                            self.serial = serial.Serial(self.port, 115200, timeout=1, write_timeout=3)
                            time.sleep(0.15)
                            self.seq = 1
                            return
                        except (serial.SerialException, OSError):
                            continue
                time.sleep(0.2)

        raise serial.SerialException("Failed to reconnect to device within timeout")

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
        for _ in range(15):
            try:
                size = self.command(CMD_HELLO, timeout=1.5)
                self.log(f"Bootloader ready; app area: {size // 1024} KiB")
                return
            except (RuntimeError, serial.SerialException):
                time.sleep(0.3)
        raise RuntimeError("SAB bootloader was not found on the selected COM port")

    def enter_from_app(self):
        self.serial.reset_input_buffer()
        self.serial.write(b"SABU")
        self.serial.flush()
        time.sleep(1.5)

    def upload(self, firmware: Path):
        image = firmware.read_bytes()
        if len(image) < 8:
            raise RuntimeError("Selected file is not an application binary")
        if len(image) > 896 * 1024:
            raise RuntimeError("Firmware is larger than the 896 KiB application partition")
        image += b"\xFF" * ((-len(image)) % 32)
        self.progress(0, len(image))
        # Ask the device to reset into bootloader without blocking for a reply.
        # Write the reset packet, close the connection and wait for the device
        # to re-enumerate (it will briefly disconnect from USB).
        # try:
        #     # ensure sequence counter starts fresh for the bootloader
        #     self.seq = 1
        #     reset_packet, _ = self.packet(CMD_RESET)
        #     try:
        #         self.serial.write(reset_packet)
        #         self.serial.flush()
        #     except serial.SerialException:
        #         # ignore — device likely disconnected immediately after reset
        #         pass
        #     try:
        #         self.serial.close()
        #     except Exception:
        #         pass
        #     # wait up to 8 seconds for the bootloader to reappear
        #     self._attempt_reconnect(8.0)
        # except Exception as exc:
        #     # If reconnect failed, surface the error so the caller can handle it
        #     raise RuntimeError(f"Failed to reset/reconnect device: {exc}")
        
        self.command(CMD_RESET, reply=False)
        time.sleep(0.5)  # give the bootloader a moment to finish initializing
        
        # Find com port again and reconnect
        self._attempt_reconnect(8.0)
        
        self.log(f"Erasing application area for {len(image):,} bytes…")
        for sector in range(7, 8):
            self.log(f"Erasing flash sector {sector}/7…")
            self.command(CMD_ERASE, bytes([sector]), timeout=30)
        self.log("Application area erased (sectors 1–7)")
        for offset in range(0, len(image), CHUNK):
            self.command(CMD_WRITE, struct.pack("<I", offset) + image[offset:offset + CHUNK])
            written = min(offset + CHUNK, len(image))
            self.progress(written, len(image))
            self.log(f"Written {written:,}/{len(image):,} bytes ({written * 100 // len(image)}%)")
        self.command(CMD_RESET)
        self.log("Update complete; device is restarting the application")


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
        values = [p.device for p in list_ports.comports()]
        self.ports["values"] = values
        if values and self.port.get() not in values:
            self.port.set(values[0])

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

            # Match either by VID/PID or by the product/manufacturer/description string.
            if (vid == 1155 and pid == 12222) or \
               ("SAB_bootloader" in manufacturer) or \
               ("SAB_bootloader" in product) or \
               ("SAB_bootloader" in description):
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
        threading.Thread(target=self.worker, daemon=True).start()

    def worker(self):
        updater = None
        try:
            path = Path(self.firmware.get())
            progress = lambda done, total: self.after(0, self.set_progress, done, total)
            updater = Updater(self.port.get(), lambda text: self.after(0, self.log, text), progress)
            try:
                updater.wait_for_bootloader()
            except RuntimeError:
                self.after(0, self.log, "Requesting update mode from running application…")
                updater.enter_from_app()
                updater.close()
                updater = None
                time.sleep(2)
                updater = Updater(self.port.get(), lambda text: self.after(0, self.log, text), progress)
                updater.wait_for_bootloader()
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
