"""Switch connection test: connect to the Switch via PABotBase2 and send a few simple commands.

    python -m pokemon_automation.switch_connection_test [port]

If `port` is omitted, every port from `list_serial_ports()` is tried in turn.
The test connects, prints the device status, presses some buttons and d-pad
steps, moves the right stick, waits for the device to confirm, and cancels with
confirmation.
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
            print(f"  status:  {sw.current_status()}")
            print(f"  name:    {sw.controller_name() or '(unknown)'}")
            sw.push_button("A")  # initial button press to wake up the connection
            sw.push_button("HOME", hold_ms=200, release_ms=1000)
            sw.push_dpad("right", hold_ms=100)
            sw.push_dpad("right", hold_ms=100)
            sw.push_dpad("left", hold_ms=100)
            sw.push_dpad("left", hold_ms=100)
            sw.push_right_joystick([1, 0], hold_ms=2000)  # hold the right joystick to the right
            sw.push_right_joystick([-1, 0], hold_ms=2000) # hold the right joystick to the left
            sw.controller_state(duration_ms=2000, buttons="B", left="up")  # hold B, while pushing up the left joystick
            sw.wait(500)
            sw.wait_for_all_requests()
            print("  cancel confirmed:", sw.cancel_all_and_wait(500))
        print("Self-test OK.")
        return True
    except Exception as e:  # noqa: BLE001 - a self-test reports, it does not crash
        print(f"  {port}: {e}")
        return False


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="python -m pokemon_automation.switch_connection_test", description=__doc__,
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
