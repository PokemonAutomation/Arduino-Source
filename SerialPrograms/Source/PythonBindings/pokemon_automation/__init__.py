"""Pokemon Automation headless console: control a Nintendo Switch from Python.

Layers (each built on the one below):

- `_pa_core` (C++, pybind11): acts as a Switch controller through a PABotBase2 serial device,
  built only from this codebase's CoreLib. Build it from SerialPrograms with
  `-DPA_PYTHON_BINDINGS=ON`.
- Video and OCR are pure Python: opencv-python for capture, pytesseract (optional)
  for OCR.
- `SwitchController`, `VideoSource`, `Console`: the Python API.
- `pokemon_automation.mcp_server`: an MCP server exposing a `Console` to AI agents.
  Run it with `python -m pokemon_automation.mcp_server --help`.

Quick start:
    from pokemon_automation import Console, list_serial_ports, list_video_devices

    print(list_serial_ports(), list_video_devices())
    with Console(serial_port="/dev/cu.usbserial-0001", video="MiraBox") as console:
        console.act([{"buttons": "A"}]).save("after_A.png")
"""

from .buttons import parse_buttons, parse_stick
from .console import Console
from .controller import InputStep, SwitchController
from .devices import list_serial_ports, list_video_devices
from .video import Frame, VideoSource, encode_image, ocr_image

__all__ = [
    "Console",
    "Frame",
    "InputStep",
    "SwitchController",
    "VideoSource",
    "encode_image",
    "list_serial_ports",
    "list_video_devices",
    "ocr_image",
    "parse_buttons",
    "parse_stick",
]

__version__ = "0.1.0"
