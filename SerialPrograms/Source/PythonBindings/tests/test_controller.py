import pytest

from pokemon_automation import Console, InputStep, SwitchController, VideoSource
from pokemon_automation import buttons as btn
from pokemon_automation.fake import FakeController, FakeVideoCapture


@pytest.fixture
def fake():
    return FakeController()


def test_press_uses_button_command(fake):
    sw = SwitchController(backend=fake)
    sw.press("A", hold_ms=50, release_ms=30, repeat=2)
    # delay = hold + release, so presses run back to back like pbf_press_button().
    assert fake.log == [("press_buttons", (80, 50, 30, btn.BUTTON_BITS["A"]))] * 2


def test_dpad_and_sticks_use_dedicated_commands(fake):
    sw = SwitchController(backend=fake)
    sw.press("down")
    sw.stick("left", "up", duration_ms=1000)
    sw.stick("right", [0.5, 0.0], duration_ms=200)
    assert fake.commands() == ["press_dpad", "move_left_joystick", "move_right_joystick"]
    name, args = [c for c in fake.log if c[0] == "move_left_joystick"][0]
    assert args == (1000, 1000, 0, 0.0, 1.0)


def test_mixed_inputs_use_full_state(fake):
    sw = SwitchController(backend=fake)
    sw.hold("B", 1500, left="up")
    name, args = fake.log[0]
    assert name == "set_state"
    assert args == (1500, btn.BUTTON_BITS["B"], btn.DPAD_NONE, 0.0, 1.0, 0.0, 0.0)


def test_sequence_is_validated_before_sending(fake):
    sw = SwitchController(backend=fake)
    with pytest.raises(ValueError):
        sw.run([{"buttons": "A"}, {"buttons": "NOT_A_BUTTON"}])
    assert fake.log == []
    with pytest.raises(ValueError, match="Unknown input step field"):
        sw.run([{"button": "A"}])


def test_sequence_duration(fake):
    sw = SwitchController(backend=fake)
    total = sw.run([
        {"buttons": "A", "hold_ms": 100, "release_ms": 50, "repeat": 3},
        {"wait_ms": 1000},
        InputStep(left_stick="up", hold_ms=500, release_ms=0),
    ])
    assert total == 3 * 150 + 1000 + 500


def test_console_act_returns_new_frame(fake):
    console = Console(controller=SwitchController(backend=fake),
                      video_source=VideoSource(backend=FakeVideoCapture(fake)))
    before = console.observe()
    after = console.act([{"buttons": "A"}], settle_ms=0)
    assert after.sequence > before.sequence
    assert after.image.shape == (180, 320, 3)
    # The fake video changes color with every command, so the input "did something".
    assert (after.image != before.image).any()
    console.stop()
    assert fake.cancel_count == 1
    console.close()
    assert console.controller is None and console.video is None


def test_closed_controller_raises(fake):
    sw = SwitchController(backend=fake)
    sw.close()
    with pytest.raises(RuntimeError, match="closed"):
        sw.press("A")


def test_step_dicts_match_mcp_schema(fake):
    """The same step dicts work in Python and in the MCP `run_inputs` tool."""
    sw = SwitchController(backend=fake)
    sw.run([{"left_stick": "up", "buttons": "B", "hold_ms": 2000, "release_ms": 0},
            {"right_stick": [0.5, 0], "hold_ms": 100}])
    assert fake.commands() == ["set_state", "move_right_joystick"]


def test_release_all_reports_confirmation(fake):
    sw = SwitchController(backend=fake)
    assert sw.release_all(100) is True
    fake.confirm_release = False
    assert sw.release_all(100) is False
    assert fake.cancel_count == 2
    sw.close()  # close() also releases
    assert fake.cancel_count == 3
