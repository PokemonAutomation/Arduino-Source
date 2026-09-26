"""Controller input vocabulary shared by the Python API and the MCP server.

Everything a caller (a script or an AI agent) types to describe an input is parsed
here, so both front ends accept exactly the same names:

- Buttons: "A", "B", "X", "Y", "L", "R", "ZL", "ZR", "PLUS" ("+", "START"),
  "MINUS" ("-", "SELECT"), "HOME", "CAPTURE", "LCLICK" ("L3", "LS"),
  "RCLICK" ("R3", "RS"). Case-insensitive.
- D-pad: "UP", "DOWN", "LEFT", "RIGHT" and diagonals such as "UP_RIGHT" (also
  "UP-RIGHT", "UPRIGHT"). Listing "UP" and "RIGHT" together also means up-right.
- Combinations: a list (["L", "R"]) or a "+"-joined string ("L+R", "ZL+A").
- Joystick positions: a direction name ("up", "down_left", ...) or an (x, y) pair in
  [-1, 1] with +x = right and +y = up. "neutral"/"center" means (0, 0).

The button bit values match `NintendoSwitch::Button` in
Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h. When the
compiled module is available, `check_against_core()` verifies that they still agree.
"""

from __future__ import annotations

import math
from collections.abc import Sequence
from dataclasses import dataclass

# Bit positions from NintendoSwitch_ControllerButtons.h.
BUTTON_BITS: dict[str, int] = {
    "Y": 1 << 0,
    "B": 1 << 1,
    "A": 1 << 2,
    "X": 1 << 3,
    "L": 1 << 4,
    "R": 1 << 5,
    "ZL": 1 << 6,
    "ZR": 1 << 7,
    "MINUS": 1 << 8,
    "PLUS": 1 << 9,
    "LCLICK": 1 << 10,
    "RCLICK": 1 << 11,
    "HOME": 1 << 12,
    "CAPTURE": 1 << 13,
}

BUTTON_ALIASES: dict[str, str] = {
    "+": "PLUS",
    "START": "PLUS",
    "-": "MINUS",
    "SELECT": "MINUS",
    "L3": "LCLICK",
    "LS": "LCLICK",
    "LSTICK": "LCLICK",
    "R3": "RCLICK",
    "RS": "RCLICK",
    "RSTICK": "RCLICK",
    "SCREENSHOT": "CAPTURE",
}

# D-pad positions from `NintendoSwitch::DpadPosition`: 0 = up, clockwise, 8 = none.
DPAD_POSITIONS: dict[str, int] = {
    "UP": 0,
    "UP_RIGHT": 1,
    "RIGHT": 2,
    "DOWN_RIGHT": 3,
    "DOWN": 4,
    "DOWN_LEFT": 5,
    "LEFT": 6,
    "UP_LEFT": 7,
}
DPAD_NONE = 8

# Unit vectors for the 8 directions, used for both the d-pad and joysticks.
_DIRECTION_VECTORS: dict[str, tuple[int, int]] = {
    "UP": (0, 1),
    "UP_RIGHT": (1, 1),
    "RIGHT": (1, 0),
    "DOWN_RIGHT": (1, -1),
    "DOWN": (0, -1),
    "DOWN_LEFT": (-1, -1),
    "LEFT": (-1, 0),
    "UP_LEFT": (-1, 1),
}

ButtonsArg = str | Sequence[str] | None
StickArg = str | Sequence[float] | None


def _normalize_name(name: str) -> str:
    name = name.strip().upper().replace("-", "_").replace(" ", "_")
    # "UPRIGHT" -> "UP_RIGHT", "DPAD_UP" -> "UP"
    if name.startswith("DPAD_"):
        name = name[5:]
    for vertical in ("UP", "DOWN"):
        for horizontal in ("LEFT", "RIGHT"):
            if name in (vertical + horizontal, horizontal + vertical, horizontal + "_" + vertical):
                return vertical + "_" + horizontal
    return name


def _split(buttons: ButtonsArg) -> list[str]:
    if buttons is None:
        return []
    if isinstance(buttons, str):
        text = buttons.strip()
        # A lone "+" or "-" is the PLUS/MINUS button, not a separator.
        if text in ("+", "-"):
            return [text]
        return [p for p in text.replace(",", "+").split("+") if p.strip()]
    ret: list[str] = []
    for item in buttons:
        ret.extend(_split(item))
    return ret


@dataclass(frozen=True)
class ParsedButtons:
    """The result of parsing a button combination."""

    bitfield: int
    dpad: int  # DPAD_NONE if no d-pad direction was given

    @property
    def has_buttons(self) -> bool:
        return self.bitfield != 0

    @property
    def has_dpad(self) -> bool:
        return self.dpad != DPAD_NONE


def parse_buttons(buttons: ButtonsArg) -> ParsedButtons:
    """Parse a button combination into a bitfield plus a d-pad position.

    Examples:
        parse_buttons("A")            -> bitfield A, no d-pad
        parse_buttons("L+R")          -> bitfield L|R
        parse_buttons(["ZL", "up"])   -> bitfield ZL, d-pad up
        parse_buttons("up+right")     -> d-pad up-right

    Raises ValueError for unknown names or contradictory d-pad directions
    (e.g. "up+down").
    """
    bitfield = 0
    dx = dy = 0
    seen_dpad: list[str] = []
    for raw in _split(buttons):
        name = raw.strip()
        key = name.upper() if name in ("+", "-") else _normalize_name(name)
        key = BUTTON_ALIASES.get(key, key)
        if key in BUTTON_BITS:
            bitfield |= BUTTON_BITS[key]
            continue
        if key in _DIRECTION_VECTORS:
            vx, vy = _DIRECTION_VECTORS[key]
            if (vx and dx and vx != dx) or (vy and dy and vy != dy):
                raise ValueError(f"Contradictory d-pad directions: {seen_dpad + [name]}")
            dx = vx or dx
            dy = vy or dy
            seen_dpad.append(name)
            continue
        raise ValueError(
            f"Unknown button {name!r}. Valid buttons: {', '.join(BUTTON_BITS)}, "
            f"d-pad: {', '.join(DPAD_POSITIONS)}."
        )
    dpad = DPAD_NONE
    if dx or dy:
        for direction, (vx, vy) in _DIRECTION_VECTORS.items():
            if (vx, vy) == (dx, dy):
                dpad = DPAD_POSITIONS[direction]
    return ParsedButtons(bitfield, dpad)


def parse_stick(position: StickArg) -> tuple[float, float]:
    """Parse a joystick position into (x, y) in [-1, 1], +y = up.

    Accepts a direction name ("up", "down_left", "neutral"), or an (x, y) pair.
    Diagonal names are normalized to length 1 so they tilt the stick fully.
    Raises ValueError for unknown names or out-of-range coordinates.
    """
    if position is None:
        return (0.0, 0.0)
    if isinstance(position, str):
        key = _normalize_name(position)
        if key in ("NEUTRAL", "CENTER", "NONE"):
            return (0.0, 0.0)
        if key not in _DIRECTION_VECTORS:
            raise ValueError(
                f"Unknown stick direction {position!r}. Use one of "
                f"{', '.join(d.lower() for d in _DIRECTION_VECTORS)}, or an [x, y] pair."
            )
        vx, vy = _DIRECTION_VECTORS[key]
        length = math.hypot(vx, vy)
        return (vx / length, vy / length)
    values = list(position)
    if len(values) != 2:
        raise ValueError(f"A stick position needs exactly two numbers [x, y], got {position!r}.")
    x, y = float(values[0]), float(values[1])
    if not (-1.0 <= x <= 1.0 and -1.0 <= y <= 1.0):
        raise ValueError(f"Stick coordinates must be within [-1, 1], got ({x}, {y}).")
    return (x, y)


def button_names(bitfield: int) -> list[str]:
    """Inverse of the bitfield part of `parse_buttons()`, for logging."""
    return [name for name, bit in BUTTON_BITS.items() if bitfield & bit]


def dpad_name(position: int) -> str | None:
    for name, value in DPAD_POSITIONS.items():
        if value == position:
            return name
    return None


def check_against_core(core_buttons: dict[str, int]) -> None:
    """Raise AssertionError if `BUTTON_BITS` disagrees with the C++ enum.

    `core_buttons` is `_pa_core.BUTTONS`, generated from `NintendoSwitch::Button`.
    """
    for name, bit in BUTTON_BITS.items():
        if core_buttons.get(name) != bit:
            raise AssertionError(
                f"Button {name} is {bit:#x} in buttons.py but {core_buttons.get(name)!r} in _pa_core."
            )
