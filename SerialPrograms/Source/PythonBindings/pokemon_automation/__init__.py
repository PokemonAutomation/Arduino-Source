"""Pokemon Automation headless console: control a Nintendo Switch from Python.

Layers (each built on the one below):

- `_pa_core` (C++, pybind11): acts as a Switch controller through a PABotBase2 serial device,
  built only from this codebase's CoreLib. Build it from SerialPrograms with
  `-DPA_PYTHON_BINDINGS=ON`.
- `buttons`: the input vocabulary (button names, d-pad, sticks).
"""

from .buttons import parse_buttons, parse_stick

__all__ = [
    "parse_buttons",
    "parse_stick",
]

__version__ = "0.1.0"
