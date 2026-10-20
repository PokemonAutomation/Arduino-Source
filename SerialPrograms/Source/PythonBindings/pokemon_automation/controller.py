"""High-level Nintendo Switch controller API on top of `_pa_core.Controller`.

Commands are queued on the microcontroller and return immediately, like the
`pbf_*()` functions in the main C++ program. Call `flush()` to wait until
everything queued so far has executed, and `cancel_all_and_wait()` to drop
anything still queued and release all inputs (waiting for the device to confirm
the neutral state).

Example:
    from pokemon_automation import SwitchController

    with SwitchController("/dev/cu.usbserial-0001") as sw:
        sw.press("A")                          # tap A
        sw.press("L+R", hold_ms=200)           # press L and R together
        sw.press("up", repeat=2)               # d-pad up twice
        sw.stick("left", "up", duration_ms=1500)   # walk forward
        sw.hold("B", left="up", duration_ms=2000)  # run forward
        sw.flush()
"""

from __future__ import annotations

import threading
from typing import Protocol

from . import _core, buttons as btn

DEFAULT_HOLD_MS = 80
DEFAULT_RELEASE_MS = 80
#  Python-side defaults for a quick tap; there is no C++ constant behind them
#  (the C++ issue_buttons()/issue_dpad() take explicit durations).


class ControllerBackend(Protocol):
    """The `_pa_core.Controller` methods used here. `fake.FakeController` implements it too.

    C++ reference: `_pa_core.Controller` is a 1:1 binding of
    `NintendoSwitch::PybindSwitchProController`
    (SerialPrograms/Source/Integrations/PybindSwitchProController.h), which wraps the
    device-side command queue of `AbstractController`
    (SerialPrograms/Source/Controllers/Controller.h). The binding itself is in
    SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp.
    """

    def wait_for_ready(self, timeout_ms: int = ...) -> bool: ...
    def is_ready(self) -> bool: ...
    def current_status(self) -> str: ...
    def controller_name(self) -> str: ...
    def wait_for_all_requests(self) -> None: ...
    def cancel_all_and_wait(self, timeout_ms: int = ...) -> bool: ...
    def wait(self, duration_ms: int) -> None: ...
    def push_button(self, delay_ms: int, hold_ms: int, release_ms: int, bitfield: int) -> None: ...
    def push_dpad(self, delay_ms: int, hold_ms: int, release_ms: int, position: int) -> None: ...
    def push_left_joystick(self, delay_ms: int, hold_ms: int, release_ms: int, x: float, y: float) -> None: ...
    def push_right_joystick(self, delay_ms: int, hold_ms: int, release_ms: int, x: float, y: float) -> None: ...
    def controller_state(
        self, duration_ms: int, buttons: int, dpad: int,
        left_x: float, left_y: float, right_x: float, right_y: float,
    ) -> None: ...


def _check_nonnegative(**kwargs: int) -> None:
    for name, value in kwargs.items():
        if value < 0:
            raise ValueError(f"{name} must not be negative, got {value}.")


class SwitchController:
    """A Switch controller on a PABotBase2 device (ESP32/Pico running PA firmware).

    `port` is the serial port, e.g. "/dev/cu.usbserial-0001" on macOS,
    "/dev/ttyACM0" on Linux or "COM3" on Windows. See `devices.list_serial_ports()`.

    Pass `backend` to wrap an existing `_pa_core.Controller` or a
    `fake.FakeController` instead of opening a port.

    Raises RuntimeError if the device isn't ready within `timeout_s` seconds.
    """

    def __init__(self, port: str | None = None, *, timeout_s: float = 10.0,
                 backend: ControllerBackend | None = None):
        if backend is None:
            if port is None:
                raise ValueError("Either port or backend is required.")
            backend = _core.core().Controller(port)
        self._backend = backend
        self._lock = threading.Lock()
        self._closed = False
        self.port = port
        if not self._backend.wait_for_ready(int(timeout_s * 1000)):
            raise RuntimeError(f"Controller on {port} is not ready: {self._backend.current_status()}")

    # ---- status -------------------------------------------------------------

    @property
    def backend(self) -> ControllerBackend:
        """The wrapped `_pa_core.Controller` (or `fake.FakeController`)."""
        return self._backend

    def is_ready(self) -> bool:
        self._require_open()
        return self._backend.is_ready()

    def status(self) -> str:
        """The connection status text: device name and firmware version, or the
        error message if the connection failed.

        C++: `PybindSwitchProController::current_status()` -> `ControllerConnection::status_text()`
        (SerialPrograms/Source/Controllers/ControllerConnection.h).
        """
        self._require_open()
        return self._backend.current_status()

    def name(self) -> str:
        """Name of the active controller implementation, e.g. "Nintendo Switch: Pro Controller"."""
        self._require_open()
        return self._backend.controller_name()

    # ---- commands -------------------------------------------------------------

    def press(self, buttons: btn.ButtonsArg, hold_ms: int = DEFAULT_HOLD_MS,
              release_ms: int = DEFAULT_RELEASE_MS, repeat: int = 1) -> None:
        """Tap a button combination (and/or one d-pad direction) `repeat` times.

        `buttons`: "A", "L+R", ["ZL", "up"], ... (see `pokemon_automation.buttons`).

        C++: maps to `issue_buttons()` / `issue_dpad()` / `issue_full_controller_state()`
        on `AbstractController` (SerialPrograms/Source/Controllers/Controller.h), via
        `PybindSwitchProController` (SerialPrograms/Source/Integrations/PybindSwitchProController.h).
        `delay = hold + release` runs commands back to back, the same convention as the
        C++ class docs.
        """
        if repeat < 1:
            raise ValueError(f"repeat must be at least 1, got {repeat}.")
        _check_nonnegative(hold_ms=hold_ms, release_ms=release_ms)
        self._require_open()
        parsed = btn.parse_buttons(buttons)
        if not parsed.has_buttons and not parsed.has_dpad:
            raise ValueError("press() needs at least one button or d-pad direction.")
        b = self._backend
        delay = hold_ms + release_ms
        with self._lock:
            for _ in range(repeat):
                if parsed.has_buttons and not parsed.has_dpad:
                    b.push_button(delay, hold_ms, release_ms, parsed.bitfield)
                elif parsed.has_dpad and not parsed.has_buttons:
                    b.push_dpad(delay, hold_ms, release_ms, parsed.dpad)
                else:
                    # Buttons and d-pad held together: one full state, then release.
                    b.controller_state(hold_ms, parsed.bitfield, parsed.dpad, 0.0, 0.0, 0.0, 0.0)
                    b.wait(release_ms)

    def hold(self, buttons: btn.ButtonsArg = None, duration_ms: int = 500, *,
             left: btn.StickArg = None, right: btn.StickArg = None) -> None:
        """Hold any combination of buttons, d-pad and sticks for `duration_ms`.

        Everything not specified is released for the whole hold.
        """
        _check_nonnegative(duration_ms=duration_ms)
        self._require_open()
        parsed = btn.parse_buttons(buttons)
        left_x, left_y = btn.parse_stick(left) if left is not None else (0.0, 0.0)
        right_x, right_y = btn.parse_stick(right) if right is not None else (0.0, 0.0)
        with self._lock:
            self._backend.controller_state(
                duration_ms, parsed.bitfield, parsed.dpad,
                left_x, left_y, right_x, right_y
            )

    def stick(self, side: str, position: btn.StickArg, duration_ms: int = 500,
              release_ms: int = 0) -> None:
        """Tilt the "left" or "right" stick to `position` for `duration_ms`."""
        _check_nonnegative(duration_ms=duration_ms, release_ms=release_ms)
        if side not in ("left", "right"):
            raise ValueError('side must be "left" or "right", got ' + repr(side))
        self._require_open()
        x, y = btn.parse_stick(position)
        with self._lock:
            if side == "left":
                self._backend.push_left_joystick(duration_ms + release_ms, duration_ms, release_ms, x, y)
            else:
                self._backend.push_right_joystick(duration_ms + release_ms, duration_ms, release_ms, x, y)

    def wait(self, duration_ms: int) -> None:
        """Queue a pause of `duration_ms` with nothing pressed."""
        _check_nonnegative(duration_ms=duration_ms)
        self._require_open()
        if duration_ms > 0:
            self._backend.wait(duration_ms)

    def flush(self) -> None:
        """Block until every queued command has executed on the device.

        C++: `PybindSwitchProController::wait_for_all_requests()` ->
        `AbstractController::wait_for_all()` (SerialPrograms/Source/Controllers/Controller.h).
        """
        self._require_open()
        self._backend.wait_for_all_requests()

    def cancel_all_and_wait(self, timeout_ms: int = 500) -> bool:
        """Cancel every queued command and return the controller to the neutral state.

        Waits up to `timeout_ms` for the device to acknowledge the cancel. Returns
        True if acknowledged, False on timeout (or if the controller is not ready).
        Thread-safe: may be called from another thread while commands are being sent
        (emergency stop).

        C++: `PybindSwitchProController::cancel_all_and_wait(timeout_millis)`
        (SerialPrograms/Source/Integrations/PybindSwitchProController.h) ->
        `AbstractController::cancel_all_and_wait()` (SerialPrograms/Source/Controllers/Controller.h).
        """
        self._require_open()
        return self._backend.cancel_all_and_wait(int(timeout_ms))

    # ---- lifecycle -------------------------------------------------------------

    def close(self) -> None:
        """Stop the device (cancelling queued commands and waiting briefly for the
        neutral state) and drop the connection."""
        if self._closed:
            return
        self._closed = True
        try:
            self._backend.cancel_all_and_wait(500)
        except Exception:  # noqa: BLE001 - best effort: the connection is dropped anyway
            pass

    def __enter__(self) -> SwitchController:
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()

    def _require_open(self) -> None:
        if self._closed:
            raise RuntimeError("This controller has been closed.")
