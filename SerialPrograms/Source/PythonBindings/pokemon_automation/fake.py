"""Fake controller and video backends for testing without hardware.

`FakeController` records every command it receives. `FakeVideoCapture` produces
synthetic frames whose color changes with each controller command, so tests (and an
agent trying the MCP server with `--fake`) can see that inputs "did something".

Neither needs the compiled `_pa_core` module. Frames are encoded with opencv-python
when it is installed; without it they are always encoded as PNG.
"""

from __future__ import annotations

import struct
import threading
import time
import zlib
from typing import Any

import numpy as np



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

    def cancel_all(self) -> None:
        with self._lock:
            self.cancel_count += 1

    def release_all(self, timeout_ms: int) -> bool:
        """Cancels like `cancel_all()`. Returns `confirm_release`, which tests can set
        to False to simulate a device that doesn't confirm in time."""
        self.cancel_all()
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


def encode_png(image: np.ndarray) -> bytes:
    """Minimal pure-Python PNG encoder for RGB uint8 images."""
    height, width, _ = image.shape
    raw = b"".join(b"\x00" + image[row].tobytes() for row in range(height))

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 3))
            + chunk(b"IEND", b""))


class FakeVideoCapture:
    """Implements the `_pa_core.VideoCapture` interface with synthetic frames."""

    def __init__(self, controller: FakeController | None = None, width: int = 320, height: int = 180):
        self.device_index = -1
        self.width = width
        self.height = height
        self._controller = controller
        self._sequence = 0

    def measured_fps(self) -> float:
        return 30.0

    def is_streaming(self) -> bool:
        return True

    def _render(self) -> np.ndarray:
        count = len(self._controller.commands()) if self._controller else 0
        image = np.zeros((self.height, self.width, 3), dtype=np.uint8)
        image[:, :, 0] = np.linspace(0, 255, self.width, dtype=np.uint8)[None, :]
        image[:, :, 1] = (count * 40) % 256
        image[:, :, 2] = 128
        return image

    def snapshot(self, min_sequence: int = 0, min_timestamp_ms: int = 0, timeout_ms: int = 2000):
        self._sequence += 1
        now_ms = max(int(time.time() * 1000), min_timestamp_ms)
        return self._render(), now_ms, self._sequence

    def encode_latest(self, format: str = "jpg", box=None, max_width: int = 0, quality: int = 85) -> bytes:
        image = self._render()
        try:
            from .video import encode_image
            return encode_image(image, format, box, max_width, quality)
        except ImportError:
            return encode_png(image)

    def close(self) -> None:
        pass
