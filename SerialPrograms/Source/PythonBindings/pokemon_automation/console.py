"""`Console`: a controller and a video feed for one Switch, used together.

This is the object both the Python API and the MCP server are built around. The core
operation is `act()`: send inputs, wait until the device has executed them, let the
game react, then capture a frame that is guaranteed to be newer than the inputs.

Example:
    from pokemon_automation import Console

    with Console(serial_port="/dev/cu.usbserial-0001", video="MiraBox") as console:
        frame = console.act([{"buttons": "HOME"}], settle_ms=1000)
        frame.save("home.png")
        print(console.read_text(box=(0.0, 0.9, 1.0, 0.1)))
"""

from __future__ import annotations

import time
from collections.abc import Iterable, Mapping
from typing import Any

from .controller import InputStep, SwitchController
from .video import Box, Frame, VideoSource


class Console:
    """A Switch driven by a controller device and observed through a capture card.

    Either part is optional: without `serial_port`/`controller` the console is
    read-only, and without `video`/`video_source` it can't observe.
    """

    def __init__(self, serial_port: str | None = None, video: int | str | None = None, *,
                 width: int = 1920, height: int = 1080,
                 controller: SwitchController | None = None,
                 video_source: VideoSource | None = None,
                 controller_timeout_s: float = 10.0):
        self.controller = controller
        self.video = video_source
        try:
            if self.video is None and video is not None:
                self.video = VideoSource(video, width, height)
            if self.controller is None and serial_port is not None:
                self.controller = SwitchController(serial_port, timeout_s=controller_timeout_s)
        except BaseException:
            self.close()
            raise

    # ---- requirements -------------------------------------------------------

    def require_controller(self) -> SwitchController:
        if self.controller is None:
            raise RuntimeError("No controller is connected.")
        return self.controller

    def require_video(self) -> VideoSource:
        if self.video is None:
            raise RuntimeError("No video device is connected.")
        return self.video

    # ---- actions --------------------------------------------------------------

    def send(self, steps: Iterable[InputStep | Mapping[str, Any]], *, wait: bool = True) -> int:
        """Queue input steps; if `wait`, block until they've executed.
        Returns the total input duration in milliseconds."""
        controller = self.require_controller()
        total = controller.run(steps)
        if wait:
            controller.flush()
        return total

    def act(self, steps: Iterable[InputStep | Mapping[str, Any]], *, settle_ms: int = 500) -> Frame:
        """Send inputs, wait for them to finish, wait `settle_ms` more for the game to
        react, then return a frame captured after all of that."""
        self.send(steps, wait=True)
        return self.observe(settle_ms=settle_ms)

    def observe(self, *, settle_ms: int = 0) -> Frame:
        """Return a frame captured at least `settle_ms` from now."""
        return self.require_video().fresh_frame(settle_ms)

    def screenshot_jpeg(self, box: Box | None = None, max_width: int = 1280, quality: int = 80,
                        *, settle_ms: int = 0) -> bytes:
        """A fresh frame (captured after `settle_ms`) as JPEG bytes."""
        video = self.require_video()
        if settle_ms > 0:
            video.fresh_frame(settle_ms)
        return video.jpeg(box, max_width, quality)

    def read_text(self, box: Box | None = None, **kwargs: Any) -> str:
        """OCR the newest frame. See `video.ocr_image()` for the options."""
        return self.require_video().frame().ocr(box, **kwargs)

    def wait_until(self, predicate, timeout_s: float = 10.0, interval_ms: int = 200) -> Frame | None:
        """Poll frames until `predicate(frame)` is truthy. Returns the matching frame,
        or None on timeout. Useful for "wait for this menu to appear" in scripts."""
        deadline = time.monotonic() + timeout_s
        while True:
            frame = self.require_video().frame()
            if predicate(frame):
                return frame
            if time.monotonic() >= deadline:
                return None
            time.sleep(interval_ms / 1000)

    def stop(self) -> None:
        """Emergency stop: request that queued inputs be cancelled and everything
        released. Thread-safe and non-blocking; see `SwitchController.stop()`."""
        if self.controller is not None:
            self.controller.stop()

    def release_all(self, timeout_ms: int = 500) -> bool:
        """Emergency stop that waits up to `timeout_ms` for the device to confirm the
        neutral state. Returns True if confirmed. Thread-safe."""
        if self.controller is None:
            return False
        return self.controller.release_all(timeout_ms)

    # ---- lifetime -------------------------------------------------------------

    def close(self) -> None:
        if self.controller is not None:
            self.controller.close()
            self.controller = None
        if self.video is not None:
            self.video.close()
            self.video = None

    def __enter__(self) -> Console:
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()
