"""Video capture, image encoding and OCR, in pure Python.

This part of the package deliberately does not use the C++ codebase: capture and
encoding use opencv-python (`cv2`), and OCR uses pytesseract (optional; it needs the
`tesseract` program installed). Only controller input goes through `_pa_core`.

Frames are numpy arrays of shape (height, width, 3), dtype uint8, in RGB order.
Boxes are (x, y, width, height) as fractions of the frame, the same convention as
`ImageFloatBox` in the main C++ program, so boxes can be copied between the two.

Example:
    video = VideoSource("MiraBox")          # by name substring, or by index
    frame = video.frame()
    frame.save("screen.png")
    print(frame.ocr(box=(0.05, 0.75, 0.9, 0.2)))
"""

from __future__ import annotations

import sys
import threading
import time
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Protocol

import numpy as np

Box = tuple[float, float, float, float]

# Tesseract page segmentation modes by friendly name.
OCR_MODES = {"block": 6, "line": 7, "word": 8, "sparse": 11}


def _cv2() -> Any:
    try:
        import cv2
    except ImportError as e:
        raise ImportError("Video features need opencv-python: pip install opencv-python") from e
    return cv2


class VideoBackend(Protocol):
    """What `VideoSource` needs from a capture backend. Implemented by `CaptureThread`
    and by `fake.FakeVideoCapture`."""

    device_index: int
    width: int
    height: int

    def measured_fps(self) -> float: ...
    def is_streaming(self) -> bool: ...
    def snapshot(self, min_sequence: int = 0, min_timestamp_ms: int = 0,
                 timeout_ms: int = 2000) -> tuple[np.ndarray | None, int, int]: ...
    def encode_latest(self, format: str = "jpg", box: Box | None = None,
                      max_width: int = 0, quality: int = 85) -> bytes: ...
    def close(self) -> None: ...


def check_box(box: Box | None) -> Box | None:
    """Validate a normalized box. Raises ValueError if it is malformed."""
    if box is None:
        return None
    if len(box) != 4:
        raise ValueError("A box is [x, y, width, height] as fractions of the frame.")
    x, y, w, h = (float(v) for v in box)
    if not (0 <= x < 1 and 0 <= y < 1 and 0 < w <= 1 and 0 < h <= 1):
        raise ValueError(f"Box values must be fractions of the frame in [0, 1], got {box!r}.")
    return (x, y, w, h)


def crop(image: np.ndarray, box: Box | None) -> np.ndarray:
    """Crop `image` to a normalized box (a view, no copy). None means the whole image."""
    box = check_box(box)
    if box is None:
        return image
    height, width = image.shape[:2]
    x, y, w, h = box
    x0, y0 = min(int(x * width + 0.5), width - 1), min(int(y * height + 0.5), height - 1)
    x1 = min(max(x0 + 1, int((x + w) * width + 0.5)), width)
    y1 = min(max(y0 + 1, int((y + h) * height + 0.5)), height)
    return image[y0:y1, x0:x1]


def _encode_bgr(bgr: np.ndarray, format: str, box: Box | None, max_width: int, quality: int) -> bytes:
    cv2 = _cv2()
    image = crop(bgr, box)
    if max_width > 0 and image.shape[1] > max_width:
        height = max(1, image.shape[0] * max_width // image.shape[1])
        image = cv2.resize(image, (max_width, height), interpolation=cv2.INTER_AREA)
    if format == "png":
        ok, data = cv2.imencode(".png", image, [cv2.IMWRITE_PNG_COMPRESSION, 1])
    elif format in ("jpg", "jpeg"):
        ok, data = cv2.imencode(".jpg", image, [cv2.IMWRITE_JPEG_QUALITY, max(0, min(100, quality))])
    else:
        raise ValueError(f'Unknown image format {format!r}; use "png" or "jpg".')
    if not ok:
        raise RuntimeError("Image encoding failed.")
    return data.tobytes()


def encode_image(image: np.ndarray, format: str = "png", box: Box | None = None,
                 max_width: int = 0, quality: int = 85) -> bytes:
    """Encode an RGB image as PNG or JPEG bytes, optionally cropped and downscaled."""
    if image.ndim != 3 or image.shape[2] != 3:
        raise ValueError("Expected an RGB image: a uint8 array of shape (height, width, 3).")
    return _encode_bgr(np.ascontiguousarray(image[:, :, ::-1]), format, box, max_width, quality)


def ocr_available() -> bool:
    """True if pytesseract and the tesseract program are installed."""
    try:
        import pytesseract
        pytesseract.get_tesseract_version()
        return True
    except Exception:
        return False


def ocr_image(image: np.ndarray, box: Box | None = None, language: str = "eng",
              mode: str | int = "block", whitelist: str = "") -> str:
    """Read text in `box` of an RGB image with Tesseract (via pytesseract).

    Steps: crop to `box`, convert to grayscale, upscale small crops so lines of text
    are ~40+ px tall (the size Tesseract's LSTM model works best at), then run
    Tesseract. `mode` is "block" (a paragraph), "line" (one line), "word", "sparse"
    (scattered text, e.g. a whole menu screen) or a raw page segmentation mode number.
    `language` is a Tesseract code such as "eng", "jpn" or "eng+jpn".

    Raises ImportError if pytesseract isn't installed, or pytesseract's
    TesseractNotFoundError if the tesseract program isn't.
    """
    cv2 = _cv2()
    try:
        import pytesseract
    except ImportError as e:
        raise ImportError("OCR needs pytesseract and Tesseract: pip install pytesseract, "
                          "and install tesseract (e.g. brew install tesseract).") from e
    if isinstance(mode, str):
        if mode not in OCR_MODES:
            raise ValueError(f"mode must be one of {', '.join(OCR_MODES)} or an integer.")
        psm = OCR_MODES[mode]
    else:
        psm = int(mode)
    gray = cv2.cvtColor(np.ascontiguousarray(crop(image, box)), cv2.COLOR_RGB2GRAY)
    if gray.shape[0] < 80:
        scale = min(4.0, 80.0 / max(1, gray.shape[0]))
        gray = cv2.resize(gray, None, fx=scale, fy=scale, interpolation=cv2.INTER_CUBIC)
    config = f"--psm {psm}"
    if whitelist:
        config += f" -c tessedit_char_whitelist={whitelist}"
    return pytesseract.image_to_string(gray, lang=language, config=config).strip()


class CaptureThread:
    """Continuously reads frames from one OpenCV video device and keeps only the latest.

    Grabbing continuously (instead of reading on demand) matters because OpenCV and the
    OS buffer several frames: a frame read on demand after a pause can be a second or
    more old, so "press a button, then look" would show the screen from before the press.

    Raises RuntimeError if the device can't be opened or delivers no frames. On macOS
    that is also what happens when the app running Python has no camera permission
    (System Settings > Privacy & Security > Camera).
    """

    def __init__(self, device_index: int, width: int = 1920, height: int = 1080):
        cv2 = _cv2()
        if sys.platform == "darwin":
            api = cv2.CAP_AVFOUNDATION
        elif sys.platform == "win32":
            api = cv2.CAP_MSMF
        elif sys.platform.startswith("linux"):
            api = cv2.CAP_V4L2
        else:
            api = cv2.CAP_ANY
        self.device_index = device_index
        self._capture = cv2.VideoCapture(device_index, api)
        if not self._capture.isOpened():
            raise RuntimeError(
                f"Unable to open video device {device_index}. Check that it exists and, on "
                "macOS, that this app has camera permission "
                "(System Settings > Privacy & Security > Camera).")
        if sys.platform != "darwin":
            # Most USB capture cards only reach 1080p at full frame rate with MJPG.
            self._capture.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"MJPG"))
        self._capture.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self._capture.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
        self._capture.set(cv2.CAP_PROP_BUFFERSIZE, 1)

        self._cond = threading.Condition()
        self._frame: np.ndarray | None = None  # BGR
        self._timestamp_ms = 0
        self._sequence = 0
        self._recent: deque[float] = deque(maxlen=120)

        # Read one frame synchronously so the resolution is known and a device that
        # opens but never delivers frames is reported here.
        first = None
        for _ in range(50):
            ok, first = self._capture.read()
            if ok and first is not None:
                break
            time.sleep(0.02)
        else:
            self._capture.release()
            raise RuntimeError(f"Video device {device_index} opened but delivered no frames.")
        self.height, self.width = first.shape[:2]
        self._store(first)

        self._stopping = threading.Event()
        self._thread = threading.Thread(target=self._loop, name=f"video-{device_index}", daemon=True)
        self._thread.start()

    def _store(self, bgr: np.ndarray) -> None:
        now = time.time()
        with self._cond:
            self._frame = bgr
            self._timestamp_ms = int(now * 1000)
            self._sequence += 1
            self._recent.append(now)
            self._cond.notify_all()

    def _loop(self) -> None:
        # If the device stops delivering frames (e.g. unplugged), keep retrying;
        # `is_streaming()` reports the stall.
        while not self._stopping.is_set():
            ok, frame = self._capture.read()
            if not ok or frame is None:
                time.sleep(0.02)
                continue
            self._store(frame)

    def _latest(self, min_sequence: int, min_timestamp_ms: int, timeout_ms: int):
        with self._cond:
            self._cond.wait_for(
                lambda: self._sequence >= min_sequence and self._timestamp_ms >= min_timestamp_ms,
                timeout=timeout_ms / 1000)
            return self._frame, self._timestamp_ms, self._sequence

    def measured_fps(self) -> float:
        with self._cond:
            recent = [t for t in self._recent if t > time.time() - 3]
        if len(recent) < 2 or recent[-1] <= recent[0]:
            return 0.0
        return (len(recent) - 1) / (recent[-1] - recent[0])

    def is_streaming(self) -> bool:
        with self._cond:
            return self._frame is not None and time.time() * 1000 - self._timestamp_ms < 2000

    def snapshot(self, min_sequence: int = 0, min_timestamp_ms: int = 0, timeout_ms: int = 2000):
        frame, timestamp, sequence = self._latest(min_sequence, min_timestamp_ms, timeout_ms)
        if frame is None:
            return None, 0, 0
        return np.ascontiguousarray(frame[:, :, ::-1]), timestamp, sequence

    def encode_latest(self, format: str = "jpg", box: Box | None = None,
                      max_width: int = 0, quality: int = 85) -> bytes:
        frame, _, _ = self._latest(0, 0, 2000)
        if frame is None:
            return b""
        return _encode_bgr(frame, format, box, max_width, quality)

    def close(self) -> None:
        self._stopping.set()
        self._thread.join(timeout=2)
        self._capture.release()


@dataclass
class Frame:
    """One captured frame."""

    image: np.ndarray  # (height, width, 3) uint8 RGB
    timestamp_ms: int  # capture time, milliseconds since the Unix epoch
    sequence: int      # frame counter since the device was opened

    @property
    def width(self) -> int:
        return int(self.image.shape[1])

    @property
    def height(self) -> int:
        return int(self.image.shape[0])

    def crop(self, box: Box) -> np.ndarray:
        return crop(self.image, box)

    def encode(self, format: str = "png", box: Box | None = None,
               max_width: int = 0, quality: int = 85) -> bytes:
        return encode_image(self.image, format, box, max_width, quality)

    def save(self, path: str | Path, box: Box | None = None, max_width: int = 0) -> Path:
        """Save as PNG, or JPEG if `path` ends in .jpg/.jpeg."""
        path = Path(path)
        fmt = "jpg" if path.suffix.lower() in (".jpg", ".jpeg") else "png"
        path.write_bytes(self.encode(fmt, box, max_width, 92))
        return path

    def ocr(self, box: Box | None = None, **kwargs: Any) -> str:
        """Read text in `box`. See `ocr_image()` for the options."""
        return ocr_image(self.image, box, **kwargs)


def find_video_device(device: int | str) -> int:
    """Resolve a device index or a case-insensitive name substring to an index.

    Raises ValueError if a name matches no device or more than one.
    """
    from .devices import list_video_devices

    if isinstance(device, int):
        return device
    text = str(device).strip()
    if text.lstrip("-").isdigit():
        return int(text)
    devices = list_video_devices()
    matches = [d for d in devices if text.lower() in str(d["name"]).lower()]
    if len(matches) == 1:
        return int(matches[0]["index"])
    names = ", ".join(f'{d["index"]}: {d["name"]}' for d in devices) or "none found"
    if not matches:
        raise ValueError(f"No video device matches {text!r}. Devices: {names}")
    raise ValueError(f"{text!r} matches several video devices; use an index. Devices: {names}")


class VideoSource:
    """The latest frames from a capture card.

    `device` is an index or a name substring (see `devices.list_video_devices()`).
    Pass `backend` to use an existing backend, e.g. `fake.FakeVideoCapture`.

    Raises RuntimeError if the device can't be opened (see `CaptureThread`).
    """

    def __init__(self, device: int | str | None = None, width: int = 1920, height: int = 1080,
                 *, backend: VideoBackend | None = None):
        if backend is None:
            if device is None:
                raise ValueError("Either device or backend is required.")
            backend = CaptureThread(find_video_device(device), width, height)
        self._backend: VideoBackend | None = backend

    @property
    def backend(self) -> VideoBackend:
        if self._backend is None:
            raise RuntimeError("This video source has been closed.")
        return self._backend

    @property
    def resolution(self) -> tuple[int, int]:
        return (self.backend.width, self.backend.height)

    def fps(self) -> float:
        return self.backend.measured_fps()

    def is_streaming(self) -> bool:
        return self.backend.is_streaming()

    def frame(self, *, after_ms: int | None = None, timeout_ms: int = 2000) -> Frame:
        """Return the newest frame. If `after_ms` (epoch milliseconds) is given, wait
        up to `timeout_ms` for a frame captured at or after that time.

        Raises RuntimeError if no frame has ever been captured.
        """
        image, timestamp, sequence = self.backend.snapshot(0, after_ms or 0, timeout_ms)
        if image is None:
            raise RuntimeError("No video frame has been captured yet.")
        return Frame(image, timestamp, sequence)

    def fresh_frame(self, settle_ms: int = 0, timeout_ms: int = 2000) -> Frame:
        """Wait `settle_ms`, then return a frame captured after the wait.

        Use this after sending inputs, so the frame shows their effect rather than a
        frame that was already buffered.
        """
        if settle_ms > 0:
            time.sleep(settle_ms / 1000)
        return self.frame(after_ms=int(time.time() * 1000), timeout_ms=timeout_ms)

    def jpeg(self, box: Box | None = None, max_width: int = 1280, quality: int = 80) -> bytes:
        """The newest frame as JPEG bytes, optionally cropped and downscaled."""
        data = self.backend.encode_latest("jpg", check_box(box), max_width, quality)
        if not data:
            raise RuntimeError("No video frame has been captured yet.")
        return data

    def close(self) -> None:
        if self._backend is not None:
            self._backend.close()
            self._backend = None

    def __enter__(self) -> VideoSource:
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()
