"""Pokemon Automation headless console: control a Nintendo Switch from Python.

Layers (each built on the one below):

- `_pa_core` (C++, pybind11): acts as a Switch controller through a PABotBase2 serial device,
  built only from this codebase's CoreLib. Build it from SerialPrograms with
  `-DPA_PYTHON_BINDINGS=ON`.
- `SwitchController`: the Python API for the controller.
"""

from .buttons import parse_buttons, parse_stick
from .controller import InputStep, SwitchController

__all__ = [
    "InputStep",
    "SwitchController",
    "parse_buttons",
    "parse_stick",
]

__version__ = "0.1.0"
