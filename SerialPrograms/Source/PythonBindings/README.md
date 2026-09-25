# Python bindings and MCP server

Control a Nintendo Switch from Python, or let an AI agent control it over MCP, using
the same controller hardware as the main program (an ESP32/Pico running PABotBase2
firmware) plus a capture card.

```
 MCP server (pokemon_automation.mcp_server)      your Python scripts
                         \                          /
              pokemon_automation (Python API: Console, SwitchController, VideoSource)
                 /                                              \
   _pa_core (C++/pybind11, CoreLib only)        opencv-python / pytesseract
   serial + PABotBase2, acts as a controller    video capture, encoding, OCR
```

- **`_pa_core`** is built only from this codebase (`CoreLib`: serial port, PABotBase2
  protocol, controller scheduling). It links no Qt, OpenCV or Tesseract.
- **Vision is pure Python**: capture and encoding use `opencv-python`; OCR uses
  `pytesseract` if installed.
- **The MCP server** is a thin layer over the Python API, so every capability is
  available to scripts and to agents alike.

## Build

The module only needs CoreLib, the GUI-free core, so use the core-only build. It
needs only a C++23 compiler, CMake and Python (with pybind11, or CMake downloads it):
no Qt, OpenCV, ONNX Runtime, Tesseract or DPP.

```bash
mkdir -p build-core && cd build-core
cmake ../SerialPrograms -DPA_CORE_ONLY=ON -DPA_PYTHON_BINDINGS=ON \
      -DPython_EXECUTABLE=$(which python3) -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 10      # CoreLib, SerialProgramsCommandLine, _pa_core
```

`-DPA_PYTHON_BINDINGS=ON` also works in a full build (next to the GUI), with
`--target _pa_core`.

The built module is copied into `pokemon_automation/` automatically. Then install the
package (editable) with the extras you want:

```bash
pip install -e "SerialPrograms/Source/PythonBindings[mcp,ocr]"
```

pybind11 is taken from your environment if installed (`pip install pybind11` or
Homebrew), otherwise downloaded by CMake. OCR also needs the `tesseract` program
(`brew install tesseract`).

## Self-test

Put the Switch on the Home menu, then:

```bash
python -m pokemon_automation.selftest --serial /dev/cu.usbserial-0001 --video MiraBox
```

It lists devices, presses HOME and a few d-pad inputs, and saves screenshots to
`selftest_output/`. **macOS:** run it from Terminal or iTerm the first time. macOS only
asks for camera permission on behalf of apps that declare camera usage; processes
started from other apps may be denied silently.

## Python API

```python
from pokemon_automation import Console, list_serial_ports, list_video_devices

print(list_serial_ports(), list_video_devices())

with Console(serial_port="/dev/cu.usbserial-0001", video="MiraBox") as console:
    frame = console.act([{"buttons": "HOME"}], settle_ms=1000)   # press, wait, capture
    frame.save("home.png")

    console.send([
        {"buttons": "DOWN", "repeat": 3},                  # d-pad down x3
        {"buttons": "A"},                                  # tap A
        {"wait_ms": 1000},
        {"left_stick": "up", "buttons": "B", "hold_ms": 2000, "release_ms": 0},  # run
    ])
    print(console.read_text(box=(0.05, 0.8, 0.9, 0.15), mode="line"))
```

The controller alone, without video:

```python
from pokemon_automation import SwitchController

with SwitchController("/dev/cu.usbserial-0001") as sw:
    sw.press("A")                          # 80 ms hold, 80 ms release
    sw.press("L+R", hold_ms=200)
    sw.stick("left", "up", duration_ms=1500)
    sw.hold("ZL", 1000, right=[0.5, 0])    # anything held together
    sw.flush()                             # block until executed
```

Conventions:
- Durations are milliseconds. Inputs are queued on the device and return
  immediately; `flush()` / `Console.send(wait=True)` blocks until they've executed.
  `stop()` requests that queued inputs be cancelled and everything released, from any
  thread, without waiting. `release_all(timeout_ms)` does the same and then waits for
  the device to confirm the neutral state (returns False on timeout, e.g. when the
  Switch is asleep and the device can't execute commands).
- Buttons: `A B X Y L R ZL ZR PLUS MINUS HOME CAPTURE LCLICK RCLICK`, d-pad `UP DOWN LEFT
  RIGHT UP_RIGHT ...`; combine with `+` or a list. Aliases such as `+`, `START`, `L3`
  work too (see `buttons.py`).
- Sticks: direction names or `[x, y]` in [-1, 1], +y = up.
- Images: numpy `(height, width, 3)` uint8 RGB. Boxes: `(x, y, width, height)` as
  fractions of the frame, the same as `ImageFloatBox` in C++.
- The controller type (Pro Controller, wired, Switch 1/2) is whatever the device is set
  to; choose it once in the main program.

## MCP server

```bash
python -m pokemon_automation.mcp_server --serial /dev/cu.usbserial-0001 --video MiraBox
python -m pokemon_automation.mcp_server --fake        # no hardware, for trying it out
```

Register with Claude Code:

```bash
claude mcp add switch -- /path/to/python -m pokemon_automation.mcp_server \
    --serial /dev/cu.usbserial-0001 --video MiraBox
```

Use `--transport streamable-http --host 127.0.0.1 --port 8765` for HTTP clients.

**macOS camera permission:** the camera is granted per *app*. If an app without camera
permission (e.g. the Claude desktop app) launches the server, video fails to open. In
that case run the server from Terminal over HTTP and connect the agent to it:

```bash
# in Terminal.app
python -m pokemon_automation.mcp_server --serial /dev/cu.usbserial-0001 --video MiraBox \
    --transport streamable-http --port 8765
# then
claude mcp add --transport http switch http://127.0.0.1:8765/mcp
```

| Tool | Purpose |
|---|---|
| `switch_status`, `list_devices`, `connect` | Connection state, device discovery, (re)connect |
| `screenshot`, `wait_and_observe` | Current screen as JPEG (optionally cropped) |
| `press_buttons`, `move_stick`, `run_inputs` | Inputs, each returning a screenshot taken `settle_ms` after they finish |
| `read_text` | OCR a region of the screen |
| `release_all` | Emergency stop; works while another input call is running, and reports whether the device confirmed the neutral state |
| `get_logs` | Recent connection/input log lines |

Safety options: `--max-hold-ms` (default 10 s per step), `--max-sequence-ms` (default
60 s per call), `--read-only` (no inputs at all). Logs go to stderr (never stdout,
which carries the protocol) and optionally `--log-file`.

## Tests

```bash
cd SerialPrograms/Source/PythonBindings
pip install -e ".[test]"
pytest
```

The tests use fake devices (`pokemon_automation.fake`) and need no hardware.

## Files

- `PythonBindings.cmake`: build rules, included by `CMakeLists.txt` when
  `PA_PYTHON_BINDINGS=ON`.
- `PythonBindings_Module.cpp`: the pybind11 module `_pa_core`. It wraps
  `Source/Integrations/PybindSwitchController.*`, the GUI-free controller class in
  CoreLib (also used by `SerialProgramsCommandLine`).
- `pokemon_automation/`: the Python package (API, vision, MCP server, fakes, self-test).
