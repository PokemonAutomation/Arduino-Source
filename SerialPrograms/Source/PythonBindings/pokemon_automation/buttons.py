"""Button and d-pad names shared by the Python API.

Everything a caller types to describe an input is parsed here:

- Buttons: "A", "B", "X", "Y", "L", "R", "ZL", "ZR", "PLUS" ("+", "START"),
  "MINUS" ("-", "SELECT"), "HOME", "CAPTURE", "LCLICK" ("L3", "LS"),
  "RCLICK" ("R3", "RS"). Case-insensitive.
- D-pad: "UP", "DOWN", "LEFT", "RIGHT" and diagonals such as "UP_RIGHT" (also
  "UP-RIGHT", "UPRIGHT"). Listing "UP" and "RIGHT" together also means up-right.
- Combinations: a list (["L", "R"]) or a "+"-joined string ("L+R", "ZL+A").
- Joystick positions: an (x, y) pair in [-1, 1] with +x = right and +y = up.
  `parse_stick()` additionally accepts a direction name ("up", "down_left", ...) and
  "neutral"/"center" for convenience; `parse_stick_position()` (used by
  push_left_joystick() / push_right_joystick()) accepts only the (x, y) pair.

The button bit values match `NintendoSwitch::Button` and the d-pad values
match `NintendoSwitch::DpadPosition` in
SerialPrograms/Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h.
When the compiled module is available, `check_against_core()` verifies that
they still agree (the module exposes both enums, built by
SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp).
"""

from __future__ import annotations

import math
from collections.abc import Sequence
from dataclasses import dataclass

# Bit positions, mirroring the C++ `enum Button` in
# SerialPrograms/Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h.
# Do not edit by hand; tests/test_buttons.py checks these against the C++ enum.
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
    "R3": "RCLICK",
    "RS": "RCLICK",
}

# D-pad positions, mirroring the C++ `enum DpadPosition` in
# SerialPrograms/Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h:
# 0 = up, clockwise, 8 = none.
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

# Unit vectors for the 8 directions, used for both the d-pad and the joysticks.
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
"""
ButtonsArg
- accepts both Buttons and D-pad inputs (see above for the exact strings). 
- validated by `parse_buttons()`
"""

StickArg = str | Sequence[float] | None
"""
StickArg
- Accepts a direction name ("up", "down_left", "neutral") or an (x, y) pair. 
- validated by `parse_stick()`
"""

StickCoords = Sequence[float]
"""
StickCoords
- A strict (x, y) pair in [-1, 1], no direction names.
- validated by `parse_stick_position()` (used by push_left_joystick() / push_right_joystick())
"""


def _normalize_name(name: str) -> str:
    name = name.strip().upper().replace("-", "_").replace(" ", "_")
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
        return [part for part in text.replace(",", "+").split("+") if part.strip()]
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
    buttonstate_bitfield = 0
    dpad_x = dpad_y = 0  # dpad_x, dpad_y represents the current overall DPAD position. starts neutral
    seen_dpad: list[str] = []
    for raw in _split(buttons):
        name = raw.strip()
        key = name.upper() if name in ("+", "-") else _normalize_name(name)
        key = BUTTON_ALIASES.get(key, key)
        if key in BUTTON_BITS:  # parse as button
            buttonstate_bitfield |= BUTTON_BITS[key]
            continue
        if key in _DIRECTION_VECTORS:  # parse as dpad
            vx, vy = _DIRECTION_VECTORS[key]  # vectors of current DPAD being parsed
            if (vx and dpad_x and vx != dpad_x) or (vy and dpad_y and vy != dpad_y):  # check for contradictory dpad directions
                raise ValueError(f"Contradictory d-pad directions: {seen_dpad + [name]}")
            dpad_x = vx or dpad_x  # update dpad_x with vx, unless it's 0 
            dpad_y = vy or dpad_y  # update dpad_y with vy, unless it's 0 
            seen_dpad.append(name)
            continue
        raise ValueError(
            f"Unknown button {name!r}. Valid buttons: {', '.join(BUTTON_BITS)}, "
            f"d-pad: {', '.join(DPAD_POSITIONS)}."
        )
    dpad = DPAD_NONE
    if dpad_x or dpad_y:
        for direction, (vx, vy) in _DIRECTION_VECTORS.items():
            if (vx, vy) == (dpad_x, dpad_y):
                dpad = DPAD_POSITIONS[direction]
    return ParsedButtons(buttonstate_bitfield, dpad)


def parse_stick(position: StickArg) -> tuple[float, float]:
    """Parse a joystick position into (x, y) in [-1, 1], +y = up.

    Accepts a direction name ("up", "down_left", "neutral") or an (x, y) pair.
    Diagonal names are normalized to length 1 so they tilt the stick fully.
    Raises ValueError for unknown names or out-of-range coordinates.
    """
    if position is None:
        return (0.0, 0.0)
    if isinstance(position, str):  # parse as string
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
        return (vx / length, vy / length) # normalize to [-1, 1]
    # else parse as (x, y) pair
    return _parse_stick_pair(position)


def _parse_stick_pair(values: StickCoords) -> tuple[float, float]:
    """Parse an (x, y) pair into floats in [-1, 1]. Raises ValueError otherwise."""
    values = list(values)
    if len(values) != 2:
        raise ValueError(f"A stick position needs exactly two numbers [x, y], got {values!r}.")
    x, y = float(values[0]), float(values[1])
    if not (-1.0 <= x <= 1.0 and -1.0 <= y <= 1.0):
        raise ValueError(f"Stick coordinates must be within [-1, 1], got ({x}, {y}).")
    return (x, y)


def parse_stick_position(position: StickCoords) -> tuple[float, float]:
    """Parse a strict (x, y) joystick position, as used by push_left_joystick() /
    push_right_joystick().

    Unlike `parse_stick()`, direction names (and None) are not allowed: pass an
    (x, y) pair in [-1, 1] with +x = right and +y = up.
    """
    if position is None:
        raise ValueError("joystick_position must be an (x, y) pair, got None.")
    if isinstance(position, str):
        raise ValueError(
            f"joystick_position must be an (x, y) pair, not a string (got {position!r}). "
            "Direction names are not accepted here."
        )
    return _parse_stick_pair(position)


def check_against_core(core_buttons: dict[str, int], core_dpad: dict[str, int]) -> None:
    """Raise AssertionError if `BUTTON_BITS` / `DPAD_POSITIONS` disagree with the C++ enums.

    `core_buttons` / `core_dpad` are `_pa_core.BUTTONS` / `_pa_core.DPAD_POSITIONS`:
    the module (SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp)
    generates them from the C++ enums in
    SerialPrograms/Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h.
    """
    for name, bit in BUTTON_BITS.items():
        if core_buttons.get(name) != bit:
            raise AssertionError(f"Button {name} is {bit:#x} in buttons.py but {core_buttons.get(name)!r} in _pa_core.")
    for name, position in DPAD_POSITIONS.items():
        if core_dpad.get(name) != position:
            raise AssertionError(f"D-pad {name} is {position} in buttons.py but {core_dpad.get(name)!r} in _pa_core.")
