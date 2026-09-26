"""Discover serial ports (controller devices) and video capture devices."""

from __future__ import annotations

import glob
import json
import subprocess
import sys
from pathlib import Path

# Serial devices that are never a PABotBase2 controller.
_IGNORED_PORT_WORDS = ("bluetooth", "debug-console", "wlan", "headphones", "airpods")


def list_serial_ports() -> list[str]:
    """Return candidate serial ports for the controller device, most likely first.

    Uses pyserial if it is installed (required on Windows); otherwise scans /dev.
    On macOS only the "cu." (call-out) nodes are listed, which is what should be
    opened; the matching "tty." nodes block on open.
    """
    ports: list[str] = []
    try:
        from serial.tools import list_ports  # type: ignore[import-not-found]
        ports = [p.device for p in list_ports.comports()]
    except ImportError:
        if sys.platform == "darwin":
            ports = glob.glob("/dev/cu.*")
        elif sys.platform.startswith("linux"):
            ports = glob.glob("/dev/ttyUSB*") + glob.glob("/dev/ttyACM*")
    if sys.platform == "darwin":
        ports = [p for p in ports if not p.startswith("/dev/tty.")]
    ports = [p for p in ports if not any(w in p.lower() for w in _IGNORED_PORT_WORDS)]
    # USB-serial adapters first.
    ports.sort(key=lambda p: (not any(w in p.lower() for w in ("usb", "uart", "acm", "com")), p))
    return ports


def list_video_devices() -> list[dict[str, int | str]]:
    """Return [{"index": int, "name": str}] for every video capture device, where
    `index` is the OpenCV device index to open.

    - macOS: AVFoundation via pyobjc (`pip install pyobjc-framework-AVFoundation`),
      in the same order OpenCV uses. Without pyobjc, names come from
      `system_profiler`, whose order usually but not always matches OpenCV's.
    - Linux: /sys/class/video4linux.
    - Windows: device names aren't available without extra libraries; returns []
      (open devices by index).
    """
    if sys.platform == "darwin":
        return _list_video_devices_macos()
    if sys.platform.startswith("linux"):
        ret = []
        for node in sorted(Path("/sys/class/video4linux").glob("video*")):
            try:
                index = int(node.name[5:])
                name = (node / "name").read_text().strip()
            except (ValueError, OSError):
                continue
            ret.append({"index": index, "name": name or f"Camera {index}"})
        return sorted(ret, key=lambda d: d["index"])
    return []


def _list_video_devices_macos() -> list[dict[str, int | str]]:
    try:
        import AVFoundation  # type: ignore[import-not-found]

        # OpenCV's AVFoundation backend indexes video devices followed by muxed ones.
        devices = list(AVFoundation.AVCaptureDevice.devicesWithMediaType_(AVFoundation.AVMediaTypeVideo))
        devices += list(AVFoundation.AVCaptureDevice.devicesWithMediaType_(AVFoundation.AVMediaTypeMuxed))
        return [{"index": i, "name": str(d.localizedName())} for i, d in enumerate(devices)]
    except ImportError:
        pass
    try:
        out = subprocess.run(["system_profiler", "SPCameraDataType", "-json"],
                             capture_output=True, text=True, timeout=10).stdout
        cameras = json.loads(out).get("SPCameraDataType", [])
    except (OSError, ValueError, subprocess.TimeoutExpired):
        return []
    return [{"index": i, "name": c.get("_name", f"Camera {i}")} for i, c in enumerate(cameras)]
