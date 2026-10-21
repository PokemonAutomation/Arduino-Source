"""Pokemon Automation: send Nintendo Switch controller input from Python.

Layers (each built on the one below):

- `_pa_core` (C++, pybind11): acts as a Switch controller through a PABotBase2
  serial device, built only from this codebase's CoreLib (the module is
  SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp, wrapping
  SerialPrograms/Source/Integrations/PybindSwitchProController.h). Build it
  from SerialPrograms with `-DPA_PYTHON_BINDINGS=ON` (see the README next to
  this package).
- `SwitchController`: the Python API on top of it, with button names instead of
  bitfields. `fake.FakeController` implements the same backend interface for
  testing without hardware.

Quick start:
    from pokemon_automation import SwitchController, list_serial_ports

    print(list_serial_ports())
    with SwitchController("/dev/cu.usbserial-0001") as sw:
        sw.push_button("A")
        sw.push_dpad("up+right")
        sw.push_left_joystick(0.0, 1.0, hold_ms=1500)
        sw.wait_for_all_requests()
"""

from .buttons import (
    BUTTON_ALIASES,
    BUTTON_BITS,
    DPAD_NONE,
    DPAD_POSITIONS,
    ParsedButtons,
    check_against_core,
    parse_buttons,
    parse_stick,
)
from .controller import SwitchController
from .devices import list_serial_ports
from .fake import FakeController

__all__ = [
    "BUTTON_ALIASES",
    "BUTTON_BITS",
    "DPAD_NONE",
    "DPAD_POSITIONS",
    "FakeController",
    "ParsedButtons",
    "SwitchController",
    "check_against_core",
    "list_serial_ports",
    "parse_buttons",
    "parse_stick",
]

__version__ = "0.1.0"
