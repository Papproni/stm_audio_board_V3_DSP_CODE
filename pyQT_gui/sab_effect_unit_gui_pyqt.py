import sys
from dataclasses import dataclass, field, asdict
from typing import List
import serial
import time

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QFormLayout,
    QFrame,
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QPushButton,
    QScrollArea,
    QSlider,
    QSpinBox,
    QTextEdit,
    QVBoxLayout,
    QWidget,
    QCheckBox,
    QDial,
    QSizePolicy,
)


MODE_OPTIONS = ["normal", "A", "B"]
FX_STATE_OPTIONS = ["BYPASS", "ACTIVE", "ERROR", "UNUSED"]
PARAM_TYPE_OPTIONS = ["UNUSED", "POT", "BTN"]

# USB CDC Commands
CMD_GET_PRESET_NUM = 0x01
CMD_GET_FX_SLOTS_DATA = 0x02
CMD_GET_FX_SLOT_PARAMS = 0x03

IMPLEMENTED_FX = [
    {"name": "NONE", "color": [80, 80, 80]},
    {"name": "Flanger", "color": [0, 255, 0]},
    {"name": "MarkIIc", "color": [0, 255, 0]},
    {"name": "lpf", "color": [150, 75, 0]},
    {"name": "hpf", "color": [75, 0, 130]},
    {"name": "bandpass", "color": [75, 0, 130]},
    {"name": "SIG_GEN", "color": [10, 50, 50]},
    {"name": "Ptchdtcr", "color": [100, 100, 100]},
    {"name": "Reverb", "color": [0, 0, 255]},
    {"name": "Envelope", "color": [231, 111, 117]},
    {"name": "Octave", "color": [255, 0, 0]},
    {"name": "CUSTOM_FX", "color": [255, 0, 0]},
    {"name": "Delay", "color": [0, 0, 255]},
    {"name": "Boost", "color": [0, 200, 100]},
    {"name": "Chorus", "color": [0, 100, 50]},
    {"name": "Overdrive", "color": [0, 255, 0]},
    {"name": "Fuzz", "color": [255, 0, 0]},
    {"name": "Tremolo", "color": [0, 255, 0]},
    {"name": "Pitcshft", "color": [255, 0, 0]},
]

FX_LIBRARY = {item["name"]: item["color"] for item in IMPLEMENTED_FX}
FX_NAME_OPTIONS = [item["name"] for item in IMPLEMENTED_FX]


@dataclass
class SabFxParam:
    type_en: str = "UNUSED"
    value_u8: int = 0
    name: str = ""


@dataclass
class FxData:
    fx_state_en: str = "ACTIVE"
    name: str = "FX"
    color: List[int] = field(default_factory=lambda: [120, 120, 120])


@dataclass
class GuitarEffectModel:
    intercom_fx_data: FxData
    intercom_parameters_aun: List[SabFxParam] = field(default_factory=list)


@dataclass
class LoopModel:
    title: str
    routing: str
    effects: List[GuitarEffectModel]


@dataclass
class UnitModel:
    PresetNum: str
    Mode: str
    loops: List[LoopModel]


def make_effect(name: str, color: List[int] | None = None, state: str = "ACTIVE") -> GuitarEffectModel:
    resolved_color = color if color is not None else FX_LIBRARY.get(name, [120, 120, 120])
    default_params = [] if name == "NONE" else [
        SabFxParam("POT", 180, "GAIN"),
        SabFxParam("POT", 120, "TONE"),
        SabFxParam("BTN", 1, "ON"),
        SabFxParam("UNUSED", 0, ""),
    ]
    return GuitarEffectModel(
        intercom_fx_data=FxData(fx_state_en=state, name=name, color=resolved_color),
        intercom_parameters_aun=default_params,
    )


def create_default_model() -> UnitModel:
    return UnitModel(
        PresetNum="A1",
        Mode="normal",
        loops=[
            LoopModel("Loop 1", "Loop 2", [
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
            ]),
            LoopModel("Loop 2", "Loop 3", [
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
            ]),
            LoopModel("Loop 3", "Loop 4", [
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
            ]),
            LoopModel("Loop 4", "Physical Output 1", [
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
                make_effect("NONE", None, "UNUSED"),
            ]),
        ],
    )


class ParamWidget(QGroupBox):
    def __init__(self, param: SabFxParam, on_change):
        super().__init__("Parameter")
        self.param = param
        self.on_change = on_change

        layout = QVBoxLayout(self)
        form = QFormLayout()

        self.name_edit = QLineEdit(param.name)
        self.name_edit.setMaxLength(4)
        self.name_edit.textChanged.connect(self._emit_change)

        self.type_combo = QComboBox()
        self.type_combo.addItems(PARAM_TYPE_OPTIONS)
        self.type_combo.setCurrentText(param.type_en)
        self.type_combo.currentTextChanged.connect(self._type_changed)

        self.value_spin = QSpinBox()
        self.value_spin.setRange(0, 255)
        self.value_spin.setValue(param.value_u8)
        self.value_spin.valueChanged.connect(self._emit_change)

        self.value_slider = QSlider(Qt.Orientation.Horizontal)
        self.value_slider.setRange(0, 255)
        self.value_slider.setValue(param.value_u8)
        self.value_slider.valueChanged.connect(self._slider_changed)

        self.btn_check = QCheckBox("Button state")
        self.btn_check.setChecked(bool(param.value_u8))
        self.btn_check.toggled.connect(self._btn_changed)

        form.addRow("Name", self.name_edit)
        form.addRow("Type", self.type_combo)
        form.addRow("Value", self.value_spin)
        layout.addLayout(form)
        layout.addWidget(self.value_slider)
        layout.addWidget(self.btn_check)

        self._refresh_visibility()

    def _type_changed(self):
        self._refresh_visibility()
        self._emit_change()

    def _refresh_visibility(self):
        is_pot = self.type_combo.currentText() == "POT"
        is_btn = self.type_combo.currentText() == "BTN"
        self.value_slider.setVisible(is_pot)
        self.btn_check.setVisible(is_btn)

    def _slider_changed(self, value: int):
        self.value_spin.blockSignals(True)
        self.value_spin.setValue(value)
        self.value_spin.blockSignals(False)
        self._emit_change()

    def _btn_changed(self, checked: bool):
        self.value_spin.blockSignals(True)
        self.value_spin.setValue(1 if checked else 0)
        self.value_spin.blockSignals(False)
        self._emit_change()

    def _emit_change(self):
        self.param.name = self.name_edit.text().upper()
        self.param.type_en = self.type_combo.currentText()
        self.param.value_u8 = self.value_spin.value()
        self.on_change()


class EffectBlock(QFrame):
    def __init__(self, loop_index: int, slot_index: int, effect: GuitarEffectModel, on_select, selected: bool = False):
        super().__init__()
        self.loop_index = loop_index
        self.slot_index = slot_index
        self.effect = effect
        self.on_select = on_select
        self.selected = selected

        self.setFrameShape(QFrame.Shape.StyledPanel)
        self.setObjectName("effectBlock")
        self.setCursor(Qt.CursorShape.PointingHandCursor)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(10, 10, 10, 10)
        layout.setSpacing(6)

        self.name_label = QLabel(self.effect.intercom_fx_data.name or "NONE")
        self.name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.name_label.setWordWrap(True)
        self.name_label.setStyleSheet("font-weight: 700; color: white;")

        self.state_label = QLabel(self.effect.intercom_fx_data.fx_state_en)
        self.state_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.state_label.setStyleSheet("font-size: 11px; color: #eeeeee;")

        layout.addStretch()
        layout.addWidget(self.name_label)
        layout.addWidget(self.state_label)
        layout.addStretch()

        self._update_style()

    def _update_style(self):
        unused = self.effect.intercom_fx_data.name == "NONE" or self.effect.intercom_fx_data.fx_state_en == "UNUSED"
        color = [90, 90, 90] if unused else FX_LIBRARY.get(self.effect.intercom_fx_data.name, [60, 60, 60])
        if unused:
            self.state_label.setText("UNUSED")
            self.name_label.setStyleSheet("font-weight: 700; color: #cccccc;")
        else:
            self.name_label.setStyleSheet("font-weight: 700; color: white;")
        border = "3px solid #ffffff" if self.selected else "2px solid #444444"
        self.setStyleSheet(
            f"background-color: rgb({color[0]}, {color[1]}, {color[2]}); border: {border}; border-radius: 12px;"
        )

    def mousePressEvent(self, event):
        if callable(self.on_select):
            self.on_select(self.loop_index, self.slot_index)
        super().mousePressEvent(event)


class ParameterGridWidget(QWidget):
    def __init__(self, params: List[SabFxParam], on_change=None):
        super().__init__()
        self.on_change = on_change
        self.params = params or []
        self.grid = QGridLayout(self)
        self.grid.setSpacing(8)
        self.grid.setContentsMargins(0, 0, 0, 0)
        self._build_grid()

    def _clear_grid(self):
        while self.grid.count():
            item = self.grid.takeAt(0)
            widget = item.widget()
            if widget is not None:
                widget.deleteLater()

    def _build_grid(self):
        self._clear_grid()
        for index in range(12):
            param = self.params[index] if index < len(self.params) else SabFxParam()
            cell = self._build_param_cell(param, index)
            row = index // 6
            col = index % 6
            self.grid.addWidget(cell, row, col)

    def _build_param_cell(self, param: SabFxParam, index: int):
        group = QGroupBox(f"P{index + 1}")
        group.setMinimumSize(160, 220)
        group.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        layout = QVBoxLayout(group)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        name_label = QLabel(param.name or "UNUSED")
        name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        name_label.setFixedHeight(24)
        name_label.setStyleSheet("font-weight: 700;")

        if param.type_en == "POT":
            value_label = QLabel(f"{param.value_u8}/255")
            value_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            dial = QDial()
            dial.setRange(0, 255)
            dial.setValue(param.value_u8)
            dial.setMinimumSize(100, 100)
            dial.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            dial.valueChanged.connect(
                lambda value, p=param, label=value_label: self._update_pot_value(p, value, label)
            )
            layout.addWidget(value_label)
            layout.addWidget(dial, alignment=Qt.AlignmentFlag.AlignCenter)
            layout.addStretch()
            layout.addWidget(name_label)
        elif param.type_en == "BTN":
            state_label = QLabel("Pressed" if param.value_u8 else "Released")
            state_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            button = QPushButton(param.name or "BTN")
            button.setCheckable(True)
            button.setChecked(param.value_u8 != 0)
            button.setMinimumSize(100, 60)
            button.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            button.clicked.connect(
                lambda checked, p=param, label=state_label: self._update_btn_value(p, checked, label)
            )
            button.setStyleSheet(
                "QPushButton { font-size: 14px; padding: 10px; } QPushButton:checked { background-color: #55aa55; }"
            )
            layout.addStretch()
            layout.addWidget(button)
            layout.addWidget(state_label)
            layout.addStretch()
            layout.addWidget(name_label)
        else:
            empty = QLabel("Unused")
            empty.setAlignment(Qt.AlignmentFlag.AlignCenter)
            empty.setStyleSheet("color: #999999;")
            layout.addStretch()
            layout.addWidget(empty)
            layout.addStretch()
            layout.addWidget(name_label)

        return group

    def _update_pot_value(self, param: SabFxParam, value: int, label: QLabel):
        param.value_u8 = value
        label.setText(f"{value}/255")
        if callable(self.on_change):
            self.on_change()

    def _update_btn_value(self, param: SabFxParam, state: int, label: QLabel):
        pressed = state != 0
        param.value_u8 = 1 if pressed else 0
        label.setText("Pressed" if pressed else "Released")
        if callable(self.on_change):
            self.on_change()

    def set_params(self, params: List[SabFxParam]):
        self.params = params or []
        self._build_grid()


class LoopWidget(QGroupBox):
    def __init__(self, loop_index: int, loop: LoopModel, routing_options: List[str], on_change, on_select, selected_effect=(0, 0)):
        super().__init__(loop.title)
        self.loop_index = loop_index
        self.loop = loop
        self.on_change = on_change
        self.on_select = on_select
        self.selected_effect = selected_effect

        root = QVBoxLayout(self)

        route_row = QHBoxLayout()
        route_row.addWidget(QLabel("Output Routing"))
        self.routing_combo = QComboBox()
        self.routing_combo.addItems(routing_options)
        self.routing_combo.setCurrentText(loop.routing)
        self.routing_combo.currentTextChanged.connect(self._emit_change)
        route_row.addWidget(self.routing_combo)
        root.addLayout(route_row)

        effects_row = QHBoxLayout()
        for slot_index, effect in enumerate(loop.effects):
            selected = self.selected_effect == (self.loop_index, slot_index)
            effects_row.addWidget(
                EffectBlock(self.loop_index, slot_index, effect, self.on_select, selected)
            )
        root.addLayout(effects_row)

    def _emit_change(self):
        self.loop.routing = self.routing_combo.currentText()
        self.on_change()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.model = create_default_model()
        self.serial_port = None
        self.selected_loop = 0
        self.selected_slot = 0
        self.connect_serial()
        self.setWindowTitle("SAB Effect Unit Editor")
        self.resize(1600, 900)

        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)

        top = QHBoxLayout()
        top.addWidget(QLabel("PresetNum"))
        self.preset_edit = QLineEdit(self.model.PresetNum)
        self.preset_edit.setMaxLength(2)
        self.preset_edit.textChanged.connect(self._top_changed)
        top.addWidget(self.preset_edit)

        top.addWidget(QLabel("Mode"))
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(MODE_OPTIONS)
        self.mode_combo.setCurrentText(self.model.Mode)
        self.mode_combo.currentTextChanged.connect(self._top_changed)
        top.addWidget(self.mode_combo)

        self.read_btn = QPushButton("Read From Unit")
        self.read_btn.clicked.connect(self.load_data_from_unit)
        top.addWidget(self.read_btn)

        self.write_btn = QPushButton("Write To Unit")
        top.addWidget(self.write_btn)
        top.addStretch()
        root.addLayout(top)

        signal_box = QGroupBox("Signal Chain Overview")
        signal_layout = QHBoxLayout(signal_box)
        self.signal_label = QLabel()
        self.signal_label.setWordWrap(True)
        signal_layout.addWidget(self.signal_label)
        root.addWidget(signal_box)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll_content = QWidget()
        self.loop_grid = QGridLayout(scroll_content)
        scroll.setWidget(scroll_content)
        root.addWidget(scroll, 1)

        detail_box = QGroupBox("Selected Effect")
        detail_layout = QVBoxLayout(detail_box)
        self.effect_title = QLabel("Select an effect")
        self.effect_title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.effect_title.setStyleSheet("font-weight: 700;")
        detail_layout.addWidget(self.effect_title)
        self.effect_detail_widget = ParameterGridWidget([])
        detail_layout.addWidget(self.effect_detail_widget)
        root.addWidget(detail_box, 1)

        preview_box = QGroupBox("Unit Data Preview")
        preview_layout = QVBoxLayout(preview_box)
        self.preview = QTextEdit()
        self.preview.setReadOnly(True)
        preview_layout.addWidget(self.preview)
        root.addWidget(preview_box, 1)

        self.setStyleSheet(
            """
            QMainWindow { background: #17181c; }
            QWidget { color: #e6e6e6; font-size: 13px; }
            QGroupBox {
                border: 1px solid #3b3f46;
                border-radius: 10px;
                margin-top: 10px;
                padding-top: 10px;
                background: #20232a;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 4px 0 4px;
            }
            QLineEdit, QComboBox, QSpinBox, QTextEdit {
                background: #111318;
                border: 1px solid #404552;
                border-radius: 6px;
                padding: 6px;
            }
            QPushButton {
                background: #2c6bed;
                border: none;
                border-radius: 8px;
                padding: 8px 14px;
                color: white;
                font-weight: 600;
            }
            QPushButton:hover { background: #3b7cff; }
            QFrame#effectCard {
                background: #161a20;
                border: 1px solid #343a45;
                border-radius: 10px;
                padding: 8px;
            }
            """
        )

        self.rebuild_loops()
        self.refresh_preview()

    def normalize_preset(self, value: str) -> str:
        value = value.strip().upper()
        if len(value) != 2:
            return value
        letter, number = value[0], value[1]
        if letter in ["A", "B", "C", "D", "E", "F", "G", "H"] and number in "123456789":
            return f"{letter}{number}"
        return value

    def _top_changed(self):
        self.model.PresetNum = self.normalize_preset(self.preset_edit.text())
        self.model.Mode = self.mode_combo.currentText()
        self.refresh_preview()

    def routing_options_for_loop(self, index: int) -> List[str]:
        if index == 0:
            return ["Loop 2 IN", "Out1"]
        if index == 1:
            return ["Loop 3 IN", "Out2"]
        if index == 2:
            return ["Loop 4 IN", "Out3"]
        return ["Out4"]

    def apply_routing_rules(self):
        if self.model.loops[0].routing == "Out1":
            self.model.loops[1].routing = "Out2"
        if self.model.loops[1].routing == "Out2":
            self.model.loops[2].routing = "Out3"
        if self.model.loops[2].routing == "Out3":
            self.model.loops[3].routing = "Out4"

    def rebuild_loops(self):
        self.apply_routing_rules()
        while self.loop_grid.count():
            item = self.loop_grid.takeAt(0)
            widget = item.widget()
            if widget is not None:
                widget.deleteLater()

        for i, loop in enumerate(self.model.loops):
            routing = self.routing_options_for_loop(i)
            if loop.routing not in routing:
                loop.routing = routing[0]
            widget = LoopWidget(
                i,
                loop,
                routing,
                self._loop_changed,
                self.select_effect,
                selected_effect=(self.selected_loop, self.selected_slot),
            )
            self.loop_grid.addWidget(widget, 0, i)

        self.refresh_chain_overview()
        self.show_selected_effect()

    def _loop_changed(self):
        self.apply_routing_rules()
        self.rebuild_loops()
        self.refresh_preview()

    def refresh_chain_overview(self):
        chain = "  |  ".join([f"{loop.title} → {loop.routing}" for loop in self.model.loops])
        self.signal_label.setText(chain)

    def refresh_preview(self):
        self.refresh_chain_overview()
        self.preview.setPlainText(str(asdict(self.model)))

    def select_effect(self, loop_index: int, slot_index: int):
        self.selected_loop = max(0, min(loop_index, len(self.model.loops) - 1))
        self.selected_slot = max(0, min(slot_index, len(self.model.loops[self.selected_loop].effects) - 1))
        self.rebuild_loops()
        self.show_selected_effect()
        self.refresh_preview()

    def show_selected_effect(self):
        effect = self.model.loops[self.selected_loop].effects[self.selected_slot]
        self.effect_title.setText(f"{effect.intercom_fx_data.name} [{effect.intercom_fx_data.fx_state_en}]")
        self.effect_detail_widget.set_params(effect.intercom_parameters_aun)

    def connect_serial(self):
        try:
            self.serial_port = serial.Serial('COM4', 115200, timeout=0.2)  # 200ms timeout for fast response
            time.sleep(0.5)  # Wait for connection
        except serial.SerialException:
            print("Could not open serial port")
            self.serial_port = None

    def send_command(self, cmd, extra_data=b''):
        if self.serial_port is None:
            return None
        data = bytes([cmd]) + extra_data
        self.serial_port.write(data)
        # time.sleep(0.001)  # 10ms minimal delay for USB CDC response
        response = self.serial_port.read(1024)  # Read up to 64 bytes
        return response

    def get_preset_num(self):
        response = self.send_command(CMD_GET_PRESET_NUM)
        if response and len(response) >= 3 and response[0] == CMD_GET_PRESET_NUM:
            major = chr(response[1])  # Already ASCII char (A, B, C, etc.)
            minor = response[2]
            return f"{major}{minor}"  # A1, B2, etc.
        return "A1"

    def get_fx_slots_data(self, loop_num):
        response = self.send_command(CMD_GET_FX_SLOTS_DATA, bytes([loop_num]))
        if response and len(response) >= 35 and response[0] == CMD_GET_FX_SLOTS_DATA:
            # response[1] is loop_num, [2],[3],[4] are states, [5:15] name1, [15:25] name2, [25:35] name3
            states = []
            names = []
            for i in range(3):
                state = response[2 + i]
                if state == 0:  # FX_STATE_ON
                    states.append("ACTIVE")
                elif state == 1:  # FX_STATE_OFF
                    states.append("BYPASS")
                else:
                    states.append("UNUSED")
                name_start = 5 + i * 10
                name_bytes = response[name_start:name_start+10]
                name = name_bytes.split(b'\x00')[0].decode('utf-8', errors='ignore')
                names.append(name)
            return states, names
        return ["UNUSED", "UNUSED", "UNUSED"], ["NONE", "NONE", "NONE"]

    def get_fx_slot_params(self, fx_slot):
        # Request all 12 parameters for this effect slot
        response = self.send_command(CMD_GET_FX_SLOT_PARAMS, bytes([fx_slot]))
        params = []
        
        if response and len(response) >= 2 and response[0] == CMD_GET_FX_SLOT_PARAMS:
            # Response format: [0]=cmd, [1]=slot, [2:]: 12 params * (type + value + name[5])
            offset = 2
            for param_idx in range(12):
                if offset + 7 <= len(response):
                    param_type = "UNUSED"
                    if response[offset] == 1:
                        param_type = "POT"
                    elif response[offset] == 2:
                        param_type = "BTN"
                    value = response[offset + 1]
                    name_bytes = response[offset + 2:offset + 7]
                    name = name_bytes.split(b'\x00')[0].decode('utf-8', errors='ignore')
                    params.append(SabFxParam(param_type, value, name))
                    offset += 7
                else:
                    params.append(SabFxParam())
        
        # Ensure we have 12 parameters
        while len(params) < 12:
            params.append(SabFxParam())
        
        return params

    def load_data_from_unit(self):
        if self.serial_port is None:
            print("Serial port not connected")
            return
        preset = self.get_preset_num()
        loops = []
        for loop_num in range(4):
            states, names = self.get_fx_slots_data(loop_num)
            effects = []
            for slot in range(3):
                fx_slot = loop_num * 3 + slot
                name = names[slot]
                color = FX_LIBRARY.get(name, [120, 120, 120])
                state = states[slot]
                if name == "NONE":
                    params = [SabFxParam("UNUSED", 0, "")]
                else:
                    # Read all 12 parameters for this effect
                    params = self.get_fx_slot_params(fx_slot)
                effect = GuitarEffectModel(
                    intercom_fx_data=FxData(fx_state_en=state, name=name, color=color),
                    intercom_parameters_aun=params
                )
                effects.append(effect)
            routing = f"Loop {loop_num+2}" if loop_num < 3 else "Physical Output 1"
            loop = LoopModel(f"Loop {loop_num+1}", routing, effects)
            loops.append(loop)
        self.model = UnitModel(PresetNum=preset, Mode="normal", loops=loops)
        self.preset_edit.setText(self.model.PresetNum)
        self.mode_combo.setCurrentText(self.model.Mode)
        self.rebuild_loops()
        self.refresh_preview()

    def load_demo_data(self):
        self.model = UnitModel(
            PresetNum="B3",
            Mode="A",
            loops=[
                LoopModel("Loop 1", "Loop 2", [
                    make_effect("BOOST", [255, 120, 50], "ACTIVE"),
                    make_effect("COMP", [120, 190, 255], "ACTIVE"),
                    make_effect("GATE", [160, 110, 255], "BYPASS"),
                ]),
                LoopModel("Loop 2", "Loop 3", [
                    make_effect("OD1", [240, 80, 80], "ACTIVE"),
                    make_effect("EQ", [80, 190, 120], "ACTIVE"),
                    make_effect("CAB", [220, 210, 90], "ACTIVE"),
                ]),
                LoopModel("Loop 3", "Loop 4", [
                    make_effect("CHR", [80, 160, 255], "BYPASS"),
                    make_effect("DLY", [70, 200, 170], "ACTIVE"),
                    make_effect("REV", [170, 130, 255], "ACTIVE"),
                ]),
                LoopModel("Loop 4", "Physical Output 1", [
                    make_effect("VOL", [200, 200, 200], "ACTIVE"),
                    make_effect("LIM", [255, 160, 120], "BYPASS"),
                    make_effect("OUT", [120, 255, 160], "ACTIVE"),
                ]),
            ],
        )
        self.preset_edit.setText(self.model.PresetNum)
        self.mode_combo.setCurrentText(self.model.Mode)
        self.rebuild_loops()
        self.refresh_preview()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
