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
    instead of bitfields (`push_button("L+R")`, `push_dpad("up")`, ...). Its
    command methods match the names of `_pa_core.Controller`.
  - `buttons.py` — parsing of button / d-pad / stick names into bitfields.
  - `fake.py` — `FakeController`: the same backend interface without hardware,
    for unit tests.
  - `devices.py` — `list_serial_ports()`.
  - `switch_connection_test.py` — `python -m pokemon_automation.switch_connection_test [port]`: connect to
    a real device and send a few commands.
- `tests/` — pytest suite (pure-Python parts always run; compiled-module tests
  skip when `_pa_core` is not built).

## Building the pybind module

Requires a Python 3.10+ with development headers and pybind11 (a system package
or `pip install pybind11`). If pybind11 is not found, CMake fails with
installation instructions — it is not downloaded automatically.

Run the following commands:

```sh
# starting from the root Arduino-Source
cd SerialPrograms
# create the build folder and cd into it
mkdir -p bin
cd bin
cmake .. -DPA_PYTHON_BINDINGS=ON -DPA_CORE_ONLY=ON
cmake --build . --target _pa_core
```

Note: If cmake has issues with finding python, you can use the command:

`cmake .. -DPA_PYTHON_BINDINGS=ON -DPA_CORE_ONLY=ON -DPython_EXECUTABLE=$(which python3)`

However, note that the `$(which ...)` command does not work in Windows.

`-DPA_CORE_ONLY=ON` builds only the GUI-free core (CoreLib,
SerialProgramsCommandLine and the module) — the fast path; without it, the
module is built alongside the full GUI program, meaning that you would have
to configure your build to include Qt and other libraries.

## Installing the pybind module

After the build, `_pa_core` is copied into
`SerialPrograms/Source/PythonBindings/pokemon_automation/`.

To install it for a project:

```sh
# starting from the root Arduino-Source
pip install -e SerialPrograms/Source/PythonBindings
```

## Example usage: Switch Connection test

`switch_connection_test.py` will connect to the Switch via PABotBase2 and send a few simple commands.

Ensure your microcontroller is connected to your computer, and has the correct firmware flashed onto it, and is ready to connect to the Switch (e.g. Switch in the Change Grip/controller menu for the Pico W and ESP32, or that the USB cable is connecting the microcontroller to the Switch dock for the ESP32-S3).

The easiest way to do a proper setup is to establish the connection to the Switch in the Computer Control program first, while noting the port used. Follow the [setup guide](https://pokemonautomation.github.io/SetupGuide/index.html) if you're not familiar with the Computer Control setup. Then, disconnect the microcontroller by either shutting down the Computer Control program, or by setting the port to "none". Note: in order for the microcontroller to connect to the Switch via this python module, the microcontroller must NOT be connected to the Computer Control program.

Run the following commands:

```sh
cd SerialPrograms/Source/PythonBindings # starting from the root Arduino-Source
python -m pokemon_automation.switch_connection_test               # try every candidate port in turn
python -m pokemon_automation.switch_connection_test /dev/ttyACM0  # or pass a specific port
```

If `port` is omitted, every port from `list_serial_ports()` is tried in turn. 

`port` examples: "/dev/cu.usbserial-0001" on macOS, "/dev/ttyACM0" on Linux, or "COM3" on Windows.

## Running the unit tests

Pytest must be installed: `pip install pytest`

```sh
# starting from the root Arduino-Source
cd SerialPrograms/Source/PythonBindings
python -m pytest
```

Running pytest, it will look inside `pyproject.toml`, then run all the tests
within the `tests` folder.
The tests that need the compiled module are skipped when `_pa_core` has not been built.

## Controller Commands
The command methods of `SwitchController` match the names of the backend they wrap
(`_pa_core.Controller` from PythonBindings_Module.cpp, and with it the C++ 
`PybindSwitchProController` methods). Here's a brief description of some of the
controller functions:

Controller commands: queued on the microcontroller and return as soon as they are
scheduled.
- `push_button()`: presses button(s) on the Switch.
- `push_dpad()`: presses the d-pad.
- `push_left_joystick()` / `push_right_joystick()`: moves the joystick.
- `controller_state()`: simultaneously holds/presses any combination of buttons,
  d-pad and sticks.
- `wait()`: queues a pause, with nothing pressed.

Other commands:
- `wait_for_all_requests()`: Blocks code execution until all the queued commands have been executed.
- `cancel_all_and_wait()`: Drops all commands queued on the microcontroller, release all inputs and 
waits for the device to confirm the neutral state — e.g. from an emergency-stop thread.
- `close()`: runs `cancel_all_and_wait()`, then drops the connection.

See `controller.py` for more details.

See `switch_connection_test.py` for example usage.

## Notes on the command queue

As mentioned above, commands are queued on the microcontroller and return as soon as they are
scheduled. i.e. the main program doesn't pause or wait for the physical button press to finish
before moving to the next line of code. It hands the command to the microcontroller and instantly moves on.
This mirrors the `pbf_*()` functions of the C++ program. 

Example 1: 
```py
push_left_joystick(..., hold_ms=10000)
push_left_joystick(..., hold_ms=10000)
push_left_joystick(..., hold_ms=10000)
do_something_else()
```

Suppose you have 3 calls to `push_left_joystick()`, where you push the joystick
for 10 seconds each. The above 3 commands will be queued onto the microcontroller,
then immediately move onto `do_something_else()`, even when the above 3 commands
have not yet finished executing on the microcontroller. If you want
`do_something_else()` to only start once the three commands have finished
executing on the microcontroller, you need to use `wait_for_all_requests()`.

Example 2:
```py
push_left_joystick(..., hold_ms=10000)
push_left_joystick(..., hold_ms=10000)
push_left_joystick(..., hold_ms=10000)
wait_for_all_requests()  # waits for the above, then runs do_something_else() below
do_something_else()
```

## Other Notes

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
