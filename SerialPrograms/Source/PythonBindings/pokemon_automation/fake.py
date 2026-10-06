"""Fake controller backend for testing without hardware.

`FakeController` records every command it receives. It doesn't need the compiled
`_pa_core` module.
"""

from __future__ import annotations

import threading
from typing import Any



class FakeController:
    """Implements the `_pa_core.Controller` interface and records calls in `log`."""

    def __init__(self, port_name: str = "fake"):
        self.port_name = port_name
        self.log: list[tuple[str, tuple[Any, ...]]] = []
        self.cancel_count = 0
        self.confirm_release = True
        self._lock = threading.Lock()

    def wait_for_ready(self, timeout_ms: int) -> bool:
        return True

    def is_ready(self) -> bool:
        return True

    def status_text(self) -> str:
        return "Fake controller (no hardware)"

    def controller_name(self) -> str:
        return "Fake Controller"

    def _record(self, name: str, *args: Any) -> None:
        with self._lock:
            self.log.append((name, args))

    def wait_for_all(self) -> None:
        pass

    def cancel_all_commands_blocking(self, timeout_ms: int) -> bool:
        """Counts the call in `cancel_count`. Returns `confirm_release`, which tests can
        set to False to simulate a device that doesn't confirm in time."""
        with self._lock:
            self.cancel_count += 1
        return self.confirm_release

    def wait(self, duration_ms: int) -> None:
        self._record("wait", duration_ms)

    def press_buttons(self, delay_ms, hold_ms, release_ms, buttons) -> None:
        self._record("press_buttons", delay_ms, hold_ms, release_ms, buttons)

    def press_dpad(self, delay_ms, hold_ms, release_ms, position) -> None:
        self._record("press_dpad", delay_ms, hold_ms, release_ms, position)

    def move_left_joystick(self, delay_ms, hold_ms, release_ms, x, y) -> None:
        self._record("move_left_joystick", delay_ms, hold_ms, release_ms, x, y)

    def move_right_joystick(self, delay_ms, hold_ms, release_ms, x, y) -> None:
        self._record("move_right_joystick", delay_ms, hold_ms, release_ms, x, y)

    def set_state(self, duration_ms, buttons, dpad, left_x, left_y, right_x, right_y) -> None:
        self._record("set_state", duration_ms, buttons, dpad, left_x, left_y, right_x, right_y)

    def commands(self) -> list[str]:
        """Names of recorded commands, excluding waits."""
        with self._lock:
            return [name for name, _ in self.log if name != "wait"]
