import math

import pytest

from pokemon_automation import buttons as btn


def test_single_and_combined_buttons():
    assert btn.parse_buttons("A").bitfield == btn.BUTTON_BITS["A"]
    both = btn.parse_buttons("L+R")
    assert both.bitfield == btn.BUTTON_BITS["L"] | btn.BUTTON_BITS["R"]
    assert not both.has_dpad
    assert btn.parse_buttons(["zl", "a"]).bitfield == btn.BUTTON_BITS["ZL"] | btn.BUTTON_BITS["A"]


def test_aliases_and_plus_minus():
    assert btn.parse_buttons("+").bitfield == btn.BUTTON_BITS["PLUS"]
    assert btn.parse_buttons("-").bitfield == btn.BUTTON_BITS["MINUS"]
    assert btn.parse_buttons("start").bitfield == btn.BUTTON_BITS["PLUS"]
    assert btn.parse_buttons("L3").bitfield == btn.BUTTON_BITS["LCLICK"]
    assert btn.parse_buttons(["A", "+"]).bitfield == btn.BUTTON_BITS["A"] | btn.BUTTON_BITS["PLUS"]


def test_dpad():
    assert btn.parse_buttons("up").dpad == btn.DPAD_POSITIONS["UP"]
    assert btn.parse_buttons("up+right").dpad == btn.DPAD_POSITIONS["UP_RIGHT"]
    assert btn.parse_buttons("down-left").dpad == btn.DPAD_POSITIONS["DOWN_LEFT"]
    assert btn.parse_buttons("UPLEFT").dpad == btn.DPAD_POSITIONS["UP_LEFT"]
    mixed = btn.parse_buttons("ZL+DOWN")
    assert mixed.bitfield == btn.BUTTON_BITS["ZL"] and mixed.dpad == btn.DPAD_POSITIONS["DOWN"]
    assert btn.parse_buttons(None).dpad == btn.DPAD_NONE


def test_invalid_buttons():
    with pytest.raises(ValueError, match="Unknown button"):
        btn.parse_buttons("Q")
    with pytest.raises(ValueError, match="Contradictory"):
        btn.parse_buttons("up+down")


def test_sticks():
    assert btn.parse_stick("up") == (0.0, 1.0)
    assert btn.parse_stick("neutral") == (0.0, 0.0)
    x, y = btn.parse_stick("down_right")
    assert math.isclose(math.hypot(x, y), 1.0) and x > 0 and y < 0
    assert btn.parse_stick([0.5, -0.25]) == (0.5, -0.25)
    with pytest.raises(ValueError):
        btn.parse_stick([2, 0])
    with pytest.raises(ValueError):
        btn.parse_stick("sideways")


def test_matches_cpp_enum():
    core = pytest.importorskip("pokemon_automation._pa_core")
    btn.check_against_core(dict(core.BUTTONS))
