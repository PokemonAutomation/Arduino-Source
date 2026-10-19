# Python Bindings

A basic Python interface for the microcontroller, built with [pybind11](https://github.com/pybind/pybind11).
It lets Python act as a Nintendo Switch controller through a PABotBase2 serial
device, using the `NintendoSwitch::PybindSwitchProController` class from CoreLib
(`Source/Integrations/PybindSwitchController.h`).

## Layout

- `PythonBindings_Module.cpp` — the `_pa_core` pybind11 module: a thin 1:1
  binding of `PybindSwitchProController` plus logging controls and the
  `BUTTONS` / `DPAD_POSITIONS` constants.
- `PythonBindings.cmake` — builds the module (linked against CoreLib only: no
  Qt, OpenCV, ONNX Runtime or Tesseract).
- `pokemon_automation/` — the Python package on top of it:
  - `controller.py` — `SwitchController`: the user-facing API, with button names
    instead of bitfields (`press("L+R")`, `stick("left", "up")`, ...).
  - `buttons.py` — parsing of button / d-pad / stick names into bitfields.
  - `fake.py` — `FakeController`: the same backend interface without hardware,
    for unit tests.
  - `devices.py` — `list_serial_ports()`.
  - `selftest.py` — `python -m pokemon_automation.selftest [port]`: connect to
    a real device and send a few commands.
- `tests/` — pytest suite (pure-Python parts always run; compiled-module tests
  skip when `_pa_core` is not built).

## Building the module

Requires a Python 3.10+ with development headers and pybind11 (a system package
or `pip install pybind11`). If neither is found, CMake downloads a pinned
pybind11 release.

```sh
cd SerialPrograms
mkdir bin && cd bin
cmake .. -DPA_PYTHON_BINDINGS=ON [-DPA_CORE_ONLY=ON] [-DPython_EXECUTABLE=$(which python3)]
cmake --build . --target _pa_core
```

`-DPA_CORE_ONLY=ON` builds only the GUI-free core (CoreLib,
SerialProgramsCommandLine and the module) — the fast path; without it the
module is built alongside the full GUI program.

After the build, `_pa_core` is copied into `pokemon_automation/`, so the
package works straight from the source tree. To install it for a project:

```sh
pip install -e SerialPrograms/Source/PythonBindings
```

## Usage

```python
from pokemon_automation import SwitchController, list_serial_ports

print(list_serial_ports())
with SwitchController("/dev/ttyACM0") as sw:
    print(sw.status())            # device name and firmware version
    sw.press("A")                 # tap A (80 ms hold, 80 ms release)
    sw.press("L+R", hold_ms=200)  # press L and R together
    sw.press("up", repeat=2)      # d-pad up twice
    sw.stick("left", "up", duration_ms=1500)   # walk forward
    sw.hold("B", left="up", duration_ms=2000)  # run forward
    sw.wait(300)
    sw.flush()                    # block until the device executed everything
```

Commands are queued on the microcontroller and return as soon as they are
scheduled, mirroring the `pbf_*()` functions of the main program. Call
`flush()` to wait for execution, and `cancel_all_and_wait()` (thread-safe) to
drop the queue, release all inputs and wait for the device to confirm the
neutral state — e.g. from an emergency-stop thread. `close()` does this for
you before dropping the connection.

Run the tests:

```sh
cd SerialPrograms/Source/PythonBindings
python -m pytest
```

## Notes

- All durations are milliseconds.
- C++ `PokemonAutomation::Exception`s surface as Python `RuntimeError`s.
- Internal log lines (the "Pybind"-tagged connection logs) are not written to
  stdout. `_pa_core.set_log_stderr(True/False)` and
  `_pa_core.set_log_file(path)` route them to stderr and/or a file, and
  `_pa_core.recent_logs(50)` returns the most recent lines.
- The device must be configured as a Pro Controller (or a wired controller)
  with the main program; the device remembers the setting.
- PR #1519 ("Agent mcp server", draft) extends this interface with a full
  `pokemon_automation` package (MCP server, video, OCR).
