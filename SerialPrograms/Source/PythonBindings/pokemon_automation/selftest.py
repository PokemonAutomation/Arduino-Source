"""Hardware self-test: check the controller, the capture card, and that inputs reach
the Switch, saving screenshots along the way.

    python -m pokemon_automation.selftest --serial /dev/cu.usbserial-0001 --video MiraBox

Put the Switch on the Home menu first. The test only presses HOME, the d-pad and B.
On macOS, run this from Terminal/iTerm the first time so macOS can ask for camera
permission for that app.
"""

from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

from .console import Console
from .devices import list_serial_ports, list_video_devices
from .video import ocr_available


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="python -m pokemon_automation.selftest", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--serial", help="Controller serial port. Omit to skip controller tests.")
    p.add_argument("--video", help="Video device index or name. Omit to skip video tests.")
    p.add_argument("--out", default="selftest_output", help="Folder for screenshots.")
    args = p.parse_args(argv)

    print("Serial ports: ", list_serial_ports())
    print("Video devices:", list_video_devices())
    print("OCR available:", ocr_available())
    if not args.serial and not args.video:
        print("\nPass --serial and/or --video to test devices.")
        return 0

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    with Console(serial_port=args.serial, video=args.video) as console:
        if console.video is not None:
            frame = console.observe(settle_ms=500)
            print(f"Video: {frame.width}x{frame.height} at {console.video.fps():.1f} fps, "
                  f"mean color {frame.image.mean(axis=(0, 1)).round(1).tolist()}")
            print("Saved", frame.save(out / "0_start.png"))
        if console.controller is not None:
            print("Controller:", console.controller.name(), "|", console.controller.status())
            for i, (label, steps, settle) in enumerate([
                ("HOME", [{"buttons": "HOME", "hold_ms": 100}], 1500),
                ("RIGHT x2", [{"buttons": "RIGHT", "repeat": 2, "release_ms": 300}], 500),
                ("LEFT x2", [{"buttons": "LEFT", "repeat": 2, "release_ms": 300}], 500),
            ], start=1):
                started = time.time()
                console.send(steps)
                print(f"Sent {label} ({time.time() - started:.2f} s)")
                if console.video is not None:
                    frame = console.observe(settle_ms=settle)
                    print("Saved", frame.save(out / f"{i}_{label.split()[0].lower()}.png"))
        if console.video is not None and ocr_available():
            print("OCR of the whole screen (sparse):",
                  repr(console.read_text(mode="sparse")[:200]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
