"""A fake controller for testing without hardware.

`FakeController` implements the same interface as `_pa_core.Controller` and
records every command it receives, so `SwitchProController` and friends can be
exercised in unit tests. It needs no compiled module.

C++ reference: the interface it mirrors is the binding of
`NintendoSwitch::PybindSwitchProController`
(SerialPrograms/Source/Integrations/PybindSwitchProController.h) in
SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp; method names
and arguments match that C++ class, so the fake is also a check that the
Python API stays in step with the C++ interface.
"""

from __future__ import annotations

import threading
from typing import Any


class FakeController:
    """Records every command it receives. `log` holds (name, args) tuples, oldest first."""

    def __init__(self, port_name: str = "fake", ready: bool = True):
        self.port_name = port_name
        self.ready = ready
        self.log: list[tuple[str, tuple[Any, ...]]] = []
        self.cancel_count = 0
        # Tests can set this to False to simulate a device that does not acknowledge in time.
        self.confirm_cancel = True
        self._lock = threading.Lock()

    # -- _pa_core.Controller interface --------------------------------------

    def wait_for_ready(self, timeout_ms: int = 10000) -> bool:
        return self.ready

    def is_ready(self) -> bool:
        return self.ready

    def current_status(self) -> str:
        return "Fake controller (no hardware)"

    def controller_name(self) -> str:
        return "Fake Controller"

    def wait_for_all_requests(self) -> None:
        pass

    def cancel_all_and_wait(self, timeout_ms: int = 500) -> bool:
        """Counts the call in `cancel_count`. Returns `confirm_cancel`, which tests can
        set to False to simulate a device that does not acknowledge in time."""
        with self._lock:
            self.cancel_count += 1
        return self.confirm_cancel

    def _record(self, name: str, *args: Any) -> None:
        with self._lock:
            self.log.append((name, args))

    def wait(self, duration_ms: int) -> None:
        self._record("wait", duration_ms)

    def push_button(self, button_bitfield: int, delay_ms: int, hold_ms: int, release_ms: int) -> None:
        self._record("push_button", button_bitfield, delay_ms, hold_ms, release_ms)

    def push_dpad(self, dpad_position: int, delay_ms: int, hold_ms: int, release_ms: int) -> None:
        self._record("push_dpad", dpad_position, delay_ms, hold_ms, release_ms)

    def push_left_joystick(self, x: float, y: float, delay_ms: int, hold_ms: int, release_ms: int) -> None:
        self._record("push_left_joystick", x, y, delay_ms, hold_ms, release_ms)

    def push_right_joystick(self, x: float, y: float, delay_ms: int, hold_ms: int, release_ms: int) -> None:
        self._record("push_right_joystick", x, y, delay_ms, hold_ms, release_ms)

    def controller_state(
        self,
        button_bitfield: int,
        dpad_position: int,
        left_x: float,
        left_y: float,
        right_x: float,
        right_y: float,
        duration_ms: int,
    ) -> None:
        self._record("controller_state", button_bitfield, dpad_position, left_x, left_y, right_x, right_y, duration_ms)

    # -- test helpers ---------------------------------------------------------

    def commands(self) -> list[str]:
        """Names of recorded commands, excluding waits."""
        with self._lock:
            return [name for name, _ in self.log if name != "wait"]

    def arguments(self, name: str) -> list[tuple[Any, ...]]:
        """Recorded argument tuples of every command called `name`."""
        with self._lock:
            return [args for n, args in self.log if n == name]
