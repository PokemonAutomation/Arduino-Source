"""High-level Nintendo Switch controller API on top of `_pa_core.Controller`.

Commands are queued on the microcontroller and return immediately, like `pbf_*()`
functions in the main C++ program. Call `flush()` to wait until everything queued so
far has executed. `stop()` cancels anything still queued and releases all inputs; it
may be called from another thread.

Example:
    with SwitchController("/dev/cu.usbserial-0001") as sw:
        sw.press("A")                       # tap A (80 ms hold, 80 ms release)
        sw.press("L+R", hold_ms=200)        # press L and R together
        sw.press("down", repeat=3)          # d-pad down three times
        sw.stick("left", "up", duration_ms=1500)   # walk forward
        sw.hold(["B"], left="up", duration_ms=2000) # run forward
        sw.flush()
"""

from __future__ import annotations

import threading
from collections.abc import Iterable, Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any, Protocol

from . import buttons as btn
from ._core import core

DEFAULT_HOLD_MS = 80
DEFAULT_RELEASE_MS = 80


class ControllerBackend(Protocol):
    """The subset of `_pa_core.Controller` used here. `fake.FakeController` implements it too."""

    def wait_for_ready(self, timeout_ms: int) -> bool: ...
    def is_ready(self) -> bool: ...
    def status_text(self) -> str: ...
    def controller_name(self) -> str: ...
    def wait_for_all(self) -> None: ...
    def cancel_all(self) -> None: ...
    def release_all(self, timeout_ms: int) -> bool: ...
    def wait(self, duration_ms: int) -> None: ...
    def press_buttons(self, delay_ms: int, hold_ms: int, release_ms: int, buttons: int) -> None: ...
    def press_dpad(self, delay_ms: int, hold_ms: int, release_ms: int, position: int) -> None: ...
    def move_left_joystick(self, delay_ms: int, hold_ms: int, release_ms: int, x: float, y: float) -> None: ...
    def move_right_joystick(self, delay_ms: int, hold_ms: int, release_ms: int, x: float, y: float) -> None: ...
    def set_state(
        self, duration_ms: int, buttons: int, dpad: int,
        left_x: float, left_y: float, right_x: float, right_y: float,
    ) -> None: ...


@dataclass
class InputStep:
    """One step of an input sequence. This is also the schema the MCP server exposes.

    A step either waits (`wait_ms` > 0 and nothing else set) or sets an input state:
    the given `buttons` (buttons and/or d-pad directions) and stick positions are held
    together for `hold_ms`, then everything is released for `release_ms`. The whole
    step is repeated `repeat` times.

    Examples:
        InputStep(wait_ms=1000)
        InputStep(buttons="A")
        InputStep(buttons="ZL+A", hold_ms=100)
        InputStep(left_stick="up", hold_ms=2000, release_ms=0)
        InputStep(buttons="B", left_stick=[0.5, 1.0], hold_ms=1500)
    """

    buttons: btn.ButtonsArg = None
    left_stick: btn.StickArg = None
    right_stick: btn.StickArg = None
    hold_ms: int = DEFAULT_HOLD_MS
    release_ms: int = DEFAULT_RELEASE_MS
    repeat: int = 1
    wait_ms: int = 0

    @classmethod
    def from_dict(cls, data: Mapping[str, Any]) -> InputStep:
        unknown = set(data) - {f for f in cls.__dataclass_fields__}
        if unknown:
            raise ValueError(f"Unknown input step field(s): {sorted(unknown)}")
        return cls(**data)

    def is_wait(self) -> bool:
        return self.buttons in (None, "", []) and self.left_stick is None and self.right_stick is None

    def duration_ms(self) -> int:
        """Total time this step occupies on the controller."""
        if self.is_wait():
            return max(0, self.wait_ms)
        return max(1, self.repeat) * (self.hold_ms + self.release_ms) + max(0, self.wait_ms)


@dataclass
class _ParsedStep:
    step: InputStep
    pressed: btn.ParsedButtons = field(default_factory=lambda: btn.ParsedButtons(0, btn.DPAD_NONE))
    left: tuple[float, float] = (0.0, 0.0)
    right: tuple[float, float] = (0.0, 0.0)


def _parse_step(step: InputStep) -> _ParsedStep:
    if step.hold_ms < 0 or step.release_ms < 0 or step.wait_ms < 0:
        raise ValueError("Durations must not be negative.")
    if step.repeat < 1:
        raise ValueError("repeat must be at least 1.")
    if step.is_wait():
        return _ParsedStep(step)
    if step.hold_ms == 0:
        raise ValueError("hold_ms must be positive for a step that presses something.")
    return _ParsedStep(
        step,
        btn.parse_buttons(step.buttons),
        btn.parse_stick(step.left_stick) if step.left_stick is not None else (0.0, 0.0),
        btn.parse_stick(step.right_stick) if step.right_stick is not None else (0.0, 0.0),
    )


class SwitchController:
    """A Switch controller on a PABotBase2 device (ESP32/Pico running PA firmware).

    `port` is the serial port, e.g. "/dev/cu.usbserial-0001" on macOS or "COM3" on
    Windows. See `devices.list_serial_ports()`.

    Pass `backend` to wrap an existing `_pa_core.Controller` or a
    `fake.FakeController` instead of opening a port.

    Raises RuntimeError if the device isn't ready within `timeout_s` seconds.
    """

    def __init__(self, port: str | None = None, *, timeout_s: float = 10.0,
                 backend: ControllerBackend | None = None):
        if backend is None:
            if port is None:
                raise ValueError("Either port or backend is required.")
            backend = core().Controller(port)
        self._backend = backend
        self._lock = threading.Lock()
        self.port = port
        if not self._backend.wait_for_ready(int(timeout_s * 1000)):
            status = self._backend.status_text()
            raise RuntimeError(f"Controller on {port} is not ready: {status}")

    # ---- status -------------------------------------------------------------

    @property
    def backend(self) -> ControllerBackend:
        return self._backend

    def is_ready(self) -> bool:
        return self._backend.is_ready()

    def status(self) -> str:
        return self._backend.status_text()

    def name(self) -> str:
        return self._backend.controller_name()

    # ---- basic commands -----------------------------------------------------

    def press(self, buttons: btn.ButtonsArg, hold_ms: int = DEFAULT_HOLD_MS,
              release_ms: int = DEFAULT_RELEASE_MS, repeat: int = 1) -> None:
        """Tap a button combination `repeat` times. D-pad directions may be mixed in."""
        self.run([InputStep(buttons=buttons, hold_ms=hold_ms, release_ms=release_ms, repeat=repeat)])

    def stick(self, side: str, position: btn.StickArg, duration_ms: int = 500,
              release_ms: int = 0) -> None:
        """Tilt the "left" or "right" stick to `position` for `duration_ms`."""
        side = side.lower()
        if side not in ("left", "right"):
            raise ValueError('side must be "left" or "right".')
        step = InputStep(hold_ms=duration_ms, release_ms=release_ms)
        setattr(step, side + "_stick", position)
        self.run([step])

    def hold(self, buttons: btn.ButtonsArg = None, duration_ms: int = 500, *,
             left: btn.StickArg = None, right: btn.StickArg = None, release_ms: int = 0) -> None:
        """Hold any combination of buttons, d-pad and sticks for `duration_ms`."""
        self.run([InputStep(buttons=buttons, left_stick=left, right_stick=right,
                            hold_ms=duration_ms, release_ms=release_ms)])

    def wait(self, duration_ms: int) -> None:
        """Queue a pause with nothing pressed."""
        if duration_ms > 0:
            self._backend.wait(int(duration_ms))

    def run(self, steps: Iterable[InputStep | Mapping[str, Any]]) -> int:
        """Queue a sequence of steps. Returns the total duration in milliseconds.

        All steps are validated before any is sent, so a typo in step 5 doesn't leave
        the first four half-executed.
        """
        parsed = [
            _parse_step(s if isinstance(s, InputStep) else InputStep.from_dict(s))
            for s in steps
        ]
        total = 0
        with self._lock:
            for p in parsed:
                self._issue(p)
                total += p.step.duration_ms()
        return total

    def flush(self) -> None:
        """Block until every queued command has executed."""
        self._backend.wait_for_all()

    def stop(self) -> None:
        """Request that the device cancel all queued commands and release every input.

        Thread-safe and non-blocking: it returns as soon as the request is handed to
        the serial connection, usually before the device has acted on it. Use
        `release_all()` to wait for the device to confirm.
        """
        self._backend.cancel_all()

    def release_all(self, timeout_ms: int = 500) -> bool:
        """Like `stop()`, then wait up to `timeout_ms` for the device to confirm it is
        in the neutral state. Returns True if confirmed. Thread-safe."""
        return self._backend.release_all(int(timeout_ms))

    # ---- context manager ----------------------------------------------------

    def __enter__(self) -> SwitchController:
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()

    def close(self) -> None:
        """Release all inputs (waiting briefly for the device to confirm) and drop
        the connection."""
        try:
            if self._backend.is_ready():
                self._backend.release_all(500)
        finally:
            self._backend = _ClosedBackend()  # type: ignore[assignment]

    # ---- internals ----------------------------------------------------------

    def _issue(self, p: _ParsedStep) -> None:
        s = p.step
        b = self._backend
        if s.is_wait():
            self.wait(s.wait_ms)
            return
        only_buttons = p.pressed.has_buttons and not p.pressed.has_dpad \
            and s.left_stick is None and s.right_stick is None
        only_dpad = p.pressed.has_dpad and not p.pressed.has_buttons \
            and s.left_stick is None and s.right_stick is None
        only_left = not p.pressed.has_buttons and not p.pressed.has_dpad \
            and s.left_stick is not None and s.right_stick is None
        only_right = not p.pressed.has_buttons and not p.pressed.has_dpad \
            and s.left_stick is None and s.right_stick is not None
        cycle = s.hold_ms + s.release_ms
        for _ in range(s.repeat):
            # Single-kind inputs use the dedicated commands, which let the scheduler
            # enforce per-button cooldowns exactly like the main program.
            if only_buttons:
                b.press_buttons(cycle, s.hold_ms, s.release_ms, p.pressed.bitfield)
            elif only_dpad:
                b.press_dpad(cycle, s.hold_ms, s.release_ms, p.pressed.dpad)
            elif only_left:
                b.move_left_joystick(cycle, s.hold_ms, s.release_ms, *p.left)
            elif only_right:
                b.move_right_joystick(cycle, s.hold_ms, s.release_ms, *p.right)
            else:
                b.set_state(s.hold_ms, p.pressed.bitfield, p.pressed.dpad, *p.left, *p.right)
                self.wait(s.release_ms)
        self.wait(s.wait_ms)


class _ClosedBackend:
    def __getattr__(self, name: str) -> Any:
        raise RuntimeError("This controller has been closed.")


def validate_steps(steps: Sequence[InputStep | Mapping[str, Any]]) -> list[InputStep]:
    """Parse and validate steps without sending them. Raises ValueError on bad input."""
    ret = []
    for s in steps:
        step = s if isinstance(s, InputStep) else InputStep.from_dict(s)
        _parse_step(step)
        ret.append(step)
    return ret
