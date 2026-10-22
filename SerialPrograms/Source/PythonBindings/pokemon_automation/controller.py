"""High-level Nintendo Switch controller API on top of `_pa_core.Controller`.

The command methods carry the same names as the backend they wrap
(`_pa_core.Controller`, a 1:1 binding of `NintendoSwitch::PybindSwitchProController`),
but take button names instead of bitfields. Commands are queued on the
microcontroller and return immediately, like the `pbf_*()` functions in the main
C++ program. Call `wait_for_all_requests()` to wait until everything queued so
far has executed, and `cancel_all_and_wait()` to drop anything still queued and
release all inputs (waiting for the device to confirm the neutral state).

Example:
    from pokemon_automation import SwitchController

    with SwitchController("/dev/cu.usbserial-0001") as sw:
        sw.push_button("A")                              # tap A
        sw.push_button("L+R", hold_ms=200)               # press L and R together
        sw.push_dpad("up")                               # d-pad up
        sw.push_left_joystick((0.0, 1.0), hold_ms=1500)  # walk forward
        sw.controller_state("B", "up", left_joystick=(0.0, 1.0), duration_ms=2000)  # run forward
        sw.wait_for_all_requests()
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
    # Note: This is a Protocol. So, it does't affect runtime at all, it's just to provide type hints
    """The `_pa_core.Controller` methods used here. `fake.FakeController` implements it too.

    C++ reference: `_pa_core.Controller` is a pybind defined in PythonBindings_Module.cpp
    It, in turn, is a 1:1 binding of `NintendoSwitch::PybindSwitchProController`
    (SerialPrograms/Source/Integrations/PybindSwitchProController.h), which wraps the
    device-side command queue of `AbstractController`
    (SerialPrograms/Source/Controllers/Controller.h).
    """

    def wait_for_ready(self, timeout_ms: int = ...) -> bool: ...
    def is_ready(self) -> bool: ...
    def current_status(self) -> str: ...
    def controller_name(self) -> str: ...
    def wait_for_all_requests(self) -> None: ...
    def cancel_all_and_wait(self, timeout_ms: int = ...) -> bool: ...
    def wait(self, duration_ms: int) -> None: ...
    def push_button(self, button_bitfield: int, delay_ms: int, hold_ms: int, release_ms: int) -> None: ...
    def push_dpad(self, dpad_position: int, delay_ms: int, hold_ms: int, release_ms: int) -> None: ...
    def push_left_joystick(self, x: float, y: float, delay_ms: int, hold_ms: int, release_ms: int) -> None: ...
    def push_right_joystick(self, x: float, y: float, delay_ms: int, hold_ms: int, release_ms: int) -> None: ...
    def controller_state(
        self, button_bitfield: int, dpad_position: int,
        left_x: float, left_y: float, right_x: float, right_y: float,
        duration_ms: int,
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

    def current_status(self) -> str:
        """The connection status text: device name and firmware version, or the
        error message if the connection failed.

        C++: `PybindSwitchProController::current_status()` -> `ControllerConnection::status_text()`
        (SerialPrograms/Source/Controllers/ControllerConnection.h).
        """
        self._require_open()
        return self._backend.current_status()

    def controller_name(self) -> str:
        """Name of the active controller implementation, e.g. "Nintendo Switch: Pro Controller"."""
        self._require_open()
        return self._backend.controller_name()

    # ---- commands -------------------------------------------------------------

    def push_button(self, buttons: btn.ButtonsArg, hold_ms: int = DEFAULT_HOLD_MS,
                    release_ms: int = DEFAULT_RELEASE_MS) -> None:
        """Tap a button combination (d-pad directions not accepted).

        `buttons`: May be a single button, or multiple buttons. e.g. "A", "L+R", ["ZL", "B"], ...
        For d-pad input use `push_dpad()`; to hold buttons and d-pad together, use
        `controller_state()`.
        
        This function does not allow for overlapping button presses. For that, use controller_state().
        However, the pybind backend does expose `delay_ms`. If this is set to less than `hold_ms`, one may
        also overlap button presses, similar to the functions in NintendoSwitch_Commands_Superscalar.h

        C++: `AbstractController::issue_buttons()`
        (SerialPrograms/Source/Controllers/Controller.h), via
        `PybindSwitchProController::push_button()`
        (SerialPrograms/Source/Integrations/PybindSwitchProController.h).
        """
        _check_nonnegative(hold_ms=hold_ms, release_ms=release_ms)
        self._require_open()
        parsed = btn.parse_buttons(buttons)
        if parsed.has_dpad:
            raise ValueError(
                f"push_button() only accepts buttons, but {buttons!r} includes a d-pad direction. "
                "Use push_dpad() for d-pad input or controller_state() to combine them."
            )
        if not parsed.has_buttons:
            raise ValueError("push_button() needs at least one button.")
        with self._lock:
            self._backend.push_button(parsed.bitfield, hold_ms + release_ms, hold_ms, release_ms)

    def push_dpad(self, dpad: btn.ButtonsArg, hold_ms: int = DEFAULT_HOLD_MS,
                  release_ms: int = DEFAULT_RELEASE_MS) -> None:
        """Tap a d-pad direction.

        `dpad`: May be a single direction, or multiple directions as long as they are not contradictory.
        e.g. "up", "up+right" (or "UP_RIGHT"), ["up", "right"], 
        For buttons use `push_button()`; to hold buttons and d-pad together, use
        `controller_state()`.

        C++: `AbstractController::issue_dpad()`
        (SerialPrograms/Source/Controllers/Controller.h), via
        `PybindSwitchProController::push_dpad()`
        (SerialPrograms/Source/Integrations/PybindSwitchProController.h).
        """
        _check_nonnegative(hold_ms=hold_ms, release_ms=release_ms)
        self._require_open()
        parsed = btn.parse_buttons(dpad)
        if parsed.has_buttons:
            raise ValueError(
                f"push_dpad() only accepts d-pad directions, but {dpad!r} includes buttons. "
                "Use push_button() for buttons or controller_state() to combine them."
            )
        if not parsed.has_dpad:
            raise ValueError("push_dpad() needs at least one d-pad direction.")
        with self._lock:
            self._backend.push_dpad(parsed.dpad, hold_ms + release_ms, hold_ms, release_ms)

    def controller_state(self, buttons: btn.ButtonsArg = None, dpad: btn.ButtonsArg = None, *,
                         left_joystick: tuple[float, float] | None = None,
                         right_joystick: tuple[float, float] | None = None,
                         duration_ms: int = 500) -> None:
        """Hold any combination of buttons, d-pad and sticks for `duration_ms`.

        * `buttons`: button names only ("A", "L+R"), or None to release all buttons.
        * `dpad`: d-pad directions only (e.g. "up", "up+right", ["up", "right"]), or None
        to release the d-pad.
        * `left_joystick` / `right_joystick` are (x, y) tuples in [-1, 1].
        -x=left, x=0=neutral, +x=right. -y=down, y=0=neutral, +y=up.
        e.g. (0.0, 1.0) is up. None (the default) releases that stick.
        """
        _check_nonnegative(duration_ms=duration_ms)
        self._require_open()
        parsed_buttons = btn.parse_buttons(buttons)
        if parsed_buttons.has_dpad:
            raise ValueError(
                f"controller_state(): the buttons argument must not include d-pad "
                f"directions ({buttons!r}). Use the dpad argument instead."
            )
        parsed_dpad = btn.parse_buttons(dpad)
        if parsed_dpad.has_buttons:
            raise ValueError(
                f"controller_state(): the dpad argument must not include buttons "
                f"({dpad!r}). Use the buttons argument instead."
            )
        left_x, left_y = btn.parse_stick_pair(left_joystick) if left_joystick is not None else (0.0, 0.0)
        right_x, right_y = btn.parse_stick_pair(right_joystick) if right_joystick is not None else (0.0, 0.0)
        with self._lock:
            self._backend.controller_state(
                parsed_buttons.bitfield, parsed_dpad.dpad,
                left_x, left_y, right_x, right_y,
                duration_ms
            )

    def push_left_joystick(self, joystick_position: tuple[float, float], hold_ms: int = 500,
                          release_ms: int = 0) -> None:
        """Tilt the left stick to `joystick_position`, an (x, y) tuple in [-1, 1], for `hold_ms`
        -x=left, x=0=neutral, +x=right. -y=down, y=0=neutral, +y=up. e.g. (0.0, 1.0) is up.

        C++: `AbstractController::issue_left_joystick()`
        (SerialPrograms/Source/Controllers/Controller.h), via
        `PybindSwitchProController::push_left_joystick()`.
        """
        self._push_joystick(self._backend.push_left_joystick, joystick_position, hold_ms, release_ms)

    def push_right_joystick(self, joystick_position: tuple[float, float], hold_ms: int = 500,
                           release_ms: int = 0) -> None:
        """Tilt the right stick to `joystick_position`, an (x, y) tuple in [-1, 1], for `hold_ms`
        -x=left, x=0=neutral, +x=right. -y=down, y=0=neutral, +y=up. e.g. (0.0, 1.0) is up.

        C++: `AbstractController::issue_right_joystick()`
        (SerialPrograms/Source/Controllers/Controller.h), via
        `PybindSwitchProController::push_right_joystick()`.
        """
        self._push_joystick(self._backend.push_right_joystick, joystick_position, hold_ms, release_ms)

    def _push_joystick(self, send, joystick_position: tuple[float, float], hold_ms: int, release_ms: int) -> None:
        _check_nonnegative(hold_ms=hold_ms, release_ms=release_ms)
        self._require_open()
        x, y = btn.parse_stick_pair(joystick_position)
        with self._lock:
            send(x, y, hold_ms + release_ms, hold_ms, release_ms)

    def wait(self, duration_ms: int) -> None:
        """Queue a pause of `duration_ms` with nothing pressed."""
        _check_nonnegative(duration_ms=duration_ms)
        self._require_open()
        if duration_ms > 0:
            self._backend.wait(duration_ms)

    def wait_for_all_requests(self) -> None:
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
