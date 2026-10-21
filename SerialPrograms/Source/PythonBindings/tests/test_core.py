#  Tests for the compiled `_pa_core` module. They are skipped when the module
#  has not been built (it is built with -DPA_PYTHON_BINDINGS=ON).
#
#  C++ reference: `_pa_core` is built from
#  SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp and wraps
#  `NintendoSwitch::PybindSwitchProController`
#  (SerialPrograms/Source/Integrations/PybindSwitchProController.h); the
#  BUTTONS / DPAD_POSITIONS dicts it exposes are generated from the enums in
#  SerialPrograms/Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h.
import pytest

core = pytest.importorskip("pokemon_automation._pa_core")


def test_buttons_dict():
    """The module's BUTTONS dict matches the C++ Button enum bit positions for a sample of buttons, including third-party controller bits.
    It checks the module against hardcoded protocol values (e.g. A == 1<<2, PLUS == 1<<9)
    """
    # A sample of the C++ `NintendoSwitch::Button` enum, straight from the module.
    assert core.BUTTONS["A"] == 1 << 2
    assert core.BUTTONS["PLUS"] == 1 << 9
    assert core.BUTTONS["HOME"] == 1 << 12
    # Third-party controller and d-pad bits exist too.
    for name in ("GR", "GL", "C", "UP", "LEFT"):
        assert name in core.BUTTONS


def test_dpad_positions_dict():
    """The module's DPAD_POSITIONS dict matches the C++ DpadPosition enum order (0 = up, clockwise, 8 = none).
    It checks the module against hardcoded protocol values.
    """
    assert core.DPAD_POSITIONS == {
        "UP": 0, "UP_RIGHT": 1, "RIGHT": 2, "DOWN_RIGHT": 3,
        "DOWN": 4, "DOWN_LEFT": 5, "LEFT": 6, "UP_LEFT": 7, "NONE": 8,
    }


def test_python_constants_match_cpp():
    """The pure-Python constants in buttons.py agree with the C++-generated values in the module."""
    from pokemon_automation import buttons as btn
    btn.check_against_core(dict(core.BUTTONS), dict(core.DPAD_POSITIONS))


def test_log_functions():
    """log() lines are retrievable through recent_logs(), and set_log_file() raises RuntimeError for a path that cannot be opened."""
    core.log("hello from python")
    assert any("hello from python" in line for line in core.recent_logs(10))
    with pytest.raises(RuntimeError):
        core.set_log_file("/nonexistent directory/pa_core.log")
    core.set_log_file("")


def test_log_file(tmp_path):
    """set_log_file() routes log lines to a file on disk (and "" disables it again)."""
    log_file = tmp_path / "pa_core.log"
    core.set_log_file(str(log_file))
    try:
        core.log("line in file")
    finally:
        core.set_log_file("")
    text = log_file.read_text()
    assert "line in file" in text


def test_unreachable_port():
    """The not-ready path of the C++ Controller: an unreachable port never becomes ready, the status reports the error, commands raise RuntimeError instead of crashing, and wait_for_all_requests()/cancel_all_and_wait() are safe no-ops."""
    c = core.Controller("nonexistent-port-xyz")
    assert not c.wait_for_ready(500)
    assert not c.is_ready()
    assert c.current_status()  # an error message, not empty
    assert c.controller_name() == ""
    # Commands on a not-ready controller raise RuntimeError, they do not crash.
    with pytest.raises(RuntimeError):
        c.push_button(core.BUTTONS["A"], 0, 10, 10)


def test_push_joystick_rejects_string():
    """The module's x/y arguments are numbers: a direction-name string is rejected with TypeError."""
    c = core.Controller("nonexistent-port-xyz")
    with pytest.raises(TypeError):
        c.push_left_joystick("up", 0.0, 0, 10, 0)
    # These are safe no-ops when not ready.
    c.wait_for_all_requests()
    assert c.cancel_all_and_wait(100) is False
