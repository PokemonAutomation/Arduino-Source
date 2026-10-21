import math

import pytest

from pokemon_automation import buttons as btn


def test_single_and_combined_buttons():
    """tests parse_buttons():
    Ensures that single button names, "+"-joined strings and lists all parse to the right bitfield, with no d-pad set.
    """
    assert btn.parse_buttons("A").bitfield == btn.BUTTON_BITS["A"]
    both = btn.parse_buttons("L+R")
    assert both.bitfield == btn.BUTTON_BITS["L"] | btn.BUTTON_BITS["R"]
    assert not both.has_dpad
    assert btn.parse_buttons(["zl", "a"]).bitfield == btn.BUTTON_BITS["ZL"] | btn.BUTTON_BITS["A"]


def test_aliases_and_plus_minus():
    """tests parse_buttons():
    Aliases (START/SELECT, L3, ...) resolve to their buttons, and a lone "+" or "-" is the PLUS/MINUS button, not a separator.
    """
    assert btn.parse_buttons("+").bitfield == btn.BUTTON_BITS["PLUS"]
    assert btn.parse_buttons("-").bitfield == btn.BUTTON_BITS["MINUS"]
    assert btn.parse_buttons("start").bitfield == btn.BUTTON_BITS["PLUS"]
    assert btn.parse_buttons("L3").bitfield == btn.BUTTON_BITS["LCLICK"]
    assert btn.parse_buttons(["A", "+"]).bitfield == btn.BUTTON_BITS["A"] | btn.BUTTON_BITS["PLUS"]


def test_dpad():
    """tests parse_buttons():
    D-pad direction names (including diagonals and mixed spellings) map to the right DpadPosition, can be combined with buttons, and None means no d-pad.
    """
    assert btn.parse_buttons("up").dpad == btn.DPAD_POSITIONS["UP"]
    assert btn.parse_buttons("up+right").dpad == btn.DPAD_POSITIONS["UP_RIGHT"]
    assert btn.parse_buttons("down-left").dpad == btn.DPAD_POSITIONS["DOWN_LEFT"]
    assert btn.parse_buttons("UPLEFT").dpad == btn.DPAD_POSITIONS["UP_LEFT"]
    mixed = btn.parse_buttons("ZL+DOWN")
    assert mixed.bitfield == btn.BUTTON_BITS["ZL"] and mixed.dpad == btn.DPAD_POSITIONS["DOWN"]
    assert btn.parse_buttons(None).dpad == btn.DPAD_NONE


def test_invalid_buttons():
    """tests parse_buttons():
    Unknown button names and contradictory d-pad directions raise ValueError; None parses as no input at all.
    """
    with pytest.raises(ValueError, match="Unknown button"):
        btn.parse_buttons("Q")
    with pytest.raises(ValueError, match="Contradictory"):
        btn.parse_buttons("up+down")
    assert not btn.parse_buttons(None).has_buttons


def test_sticks():
    """parse_stick() maps direction names to unit vectors (diagonals normalized to length 1), passes raw [x, y] pairs through, and rejects out-of-range or unknown values."""
    assert btn.parse_stick("up") == (0.0, 1.0)
    assert btn.parse_stick("neutral") == (0.0, 0.0)
    x, y = btn.parse_stick("down_right")
    assert math.isclose(math.hypot(x, y), 1.0) and x > 0 and y < 0
    assert btn.parse_stick([0.5, -0.25]) == (0.5, -0.25)
    with pytest.raises(ValueError):
        btn.parse_stick([2, 0])
    with pytest.raises(ValueError):
        btn.parse_stick("sideways")

