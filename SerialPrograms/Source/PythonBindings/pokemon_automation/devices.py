"""Discover serial ports that may be a PABotBase2 controller device."""

from __future__ import annotations

import glob
import sys

# Serial devices that are never a PABotBase2 controller.
_IGNORED_PORT_WORDS = ("bluetooth", "debug-console", "wlan", "headphones", "airpods")


def list_serial_ports() -> list[str]:
    """Return candidate serial ports for the controller device, most likely first.

    Uses pyserial if it is installed (`pip install pyserial`); otherwise scans
    /dev (macOS and Linux) or enumerates COM ports by name (Windows). On macOS
    only the "cu." (call-out) nodes are listed, which are the ones to open; the
    matching "tty." nodes block on open.
    """
    ports: list[str] = []
    try:
        from serial.tools import list_ports  # type: ignore[import-not-found]
        ports = [p.device for p in list_ports.comports()]
    except ImportError:
        if sys.platform == "darwin":
            ports = glob.glob("/dev/cu.*")
        elif sys.platform.startswith("linux"):
            ports = glob.glob("/dev/ttyUSB*") + glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyS[0-9]*")
        else:
            ports = [f"COM{i}" for i in range(1, 64)]
    if sys.platform == "darwin":
        ports = [p for p in ports if not p.startswith("/dev/tty.")]
    ports = [p for p in ports if not any(w in p.lower() for w in _IGNORED_PORT_WORDS)]
    # USB-serial adapters first.
    ports.sort(key=lambda p: (not any(w in p.lower() for w in ("usb", "uart", "acm", "com")), p))
    return ports
