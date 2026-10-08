"""Tests of the compiled `_pa_core` module that don't need hardware."""

import pytest

core = pytest.importorskip("pokemon_automation._pa_core")

from pokemon_automation import buttons as btn  # noqa: E402


def test_button_bits_match_cpp_enum():
    btn.check_against_core(dict(core.BUTTONS))


def test_log_roundtrip():
    core.log("hello from the test")
    assert any("hello from the test" in line for line in core.recent_logs(10))


def test_missing_port_is_not_ready():
    controller = core.Controller("/dev/does-not-exist")
    assert controller.wait_for_ready(3000) is False
    assert not controller.is_ready()
    with pytest.raises(RuntimeError, match="not ready"):
        controller.press_buttons(80, 80, 0, btn.BUTTON_BITS["A"])
