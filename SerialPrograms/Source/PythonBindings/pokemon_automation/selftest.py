"""Hardware self-test: connect to a PABotBase2 and send a few simple commands.

    python -m pokemon_automation.selftest [port]

If `port` is omitted, every port from `list_serial_ports()` is tried in turn.
The test connects, prints the device status, presses HOME and a d-pad step,
waits for the device to confirm, and cancels. It only presses HOME and the d-pad.
"""

from __future__ import annotations

import argparse
import sys

from . import _core, devices
from .controller import SwitchController


def try_port(port: str) -> bool:
    try:
        with SwitchController(port, timeout_s=5.0) as sw:
            print(f"Connected to {port}")
            print(f"  status:  {sw.status()}")
            print(f"  name:    {sw.name() or '(unknown)'}")
            sw.press("HOME")
            sw.press("right", hold_ms=100)
            sw.wait(500)
            sw.flush()
            sw.cancel_all_commands()
        print("Self-test OK.")
        return True
    except Exception as e:  # noqa: BLE001 - a self-test reports, it does not crash
        print(f"  {port}: {e}")
        return False


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="python -m pokemon_automation.selftest", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("port", nargs="?", help="Controller serial port. Omit to try every candidate port.")
    args = p.parse_args(argv)

    if not _core.core_available():
        print(_core.BUILD_HINT, file=sys.stderr)
        return 2

    ports = [args.port] if args.port else devices.list_serial_ports()
    if not ports:
        print("No candidate serial ports found; pass one as an argument.")
        return 1

    print(f"Candidate ports: {ports}")
    for port in ports:
        if try_port(port):
            return 0
    print("Could not connect to any port.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
