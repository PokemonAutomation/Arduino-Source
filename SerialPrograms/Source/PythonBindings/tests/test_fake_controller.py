#  SwitchController on top of a FakeController (no hardware, no compiled module).
#  The command names match the backend (`_pa_core.Controller`), which mirrors the
#  C++ interface of `NintendoSwitch::PybindSwitchProController`
#  (SerialPrograms/Source/Integrations/PybindSwitchProController.h), bound by
#  SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp.
import pytest

from pokemon_automation import BUTTON_BITS, DPAD_NONE, DPAD_POSITIONS, FakeController, SwitchController


def make():
    """Create a SwitchController on a fresh FakeController and return both, so tests can inspect the recorded commands."""
    fake = FakeController()
    sw = SwitchController(backend=fake)
    return sw, fake


def test_requires_port_or_backend():
    """Constructing a SwitchController without a port or backend raises ValueError."""
    with pytest.raises(ValueError):
        SwitchController()


def test_not_ready_raises():
    """The constructor raises RuntimeError when the backend is not ready within the timeout."""
    fake = FakeController(ready=False)
    with pytest.raises(RuntimeError, match="not ready"):
        SwitchController(backend=fake, timeout_s=0.1)


def test_status_and_name_match_backend():
    """current_status() and controller_name() pass the backend's values through unchanged."""
    sw, fake = make()
    assert sw.current_status() == fake.current_status()
    assert sw.controller_name() == fake.controller_name()


def test_push_button_single_button():
    """Tapping one button issues a backend push_button with the right bitfield and the default 80/80 ms timing."""
    sw, fake = make()
    sw.push_button("A")
    assert fake.commands() == ["push_button"]
    (bitfield, delay, hold, release), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["A"]
    assert delay == hold + release and hold == 80 and release == 80


def test_push_button_combined_and_custom_durations():
    """A combination is OR-ed into one bitfield and custom hold/release durations are forwarded, with delay = hold + release."""
    sw, fake = make()
    sw.push_button("L+R", hold_ms=200, release_ms=50)
    (bitfield, delay, hold, release), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["L"] | BUTTON_BITS["R"]
    assert (delay, hold, release) == (250, 200, 50)


def test_push_dpad():
    """A d-pad direction is forwarded as push_dpad with the C++ DpadPosition value and the requested timing."""
    sw, fake = make()
    sw.push_dpad("up", hold_ms=100)
    (position, delay, hold, release), = fake.arguments("push_dpad")
    assert position == DPAD_POSITIONS["UP"] and hold == 100 and delay == 100 + release


def test_push_button_rejects_dpad():
    """push_button() rejects d-pad input with a ValueError pointing to push_dpad(), and queues nothing."""
    sw, fake = make()
    with pytest.raises(ValueError, match="push_dpad"):
        sw.push_button("ZL+up")
    with pytest.raises(ValueError, match="push_dpad"):
        sw.push_button("up")
    assert fake.commands() == []


def test_push_dpad_rejects_buttons():
    """push_dpad() rejects button names with a ValueError pointing to push_button(), and queues nothing."""
    sw, fake = make()
    with pytest.raises(ValueError, match="push_button"):
        sw.push_dpad("ZL+up")
    with pytest.raises(ValueError, match="push_button"):
        sw.push_dpad("ZL")
    assert fake.commands() == []


def test_push_button_needs_something():
    """push_button(None) raises ValueError and queues no command."""
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_button(None)
    assert fake.commands() == []


def test_push_dpad_needs_something():
    """push_dpad(None) raises ValueError and queues no command."""
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_dpad(None)
    assert fake.commands() == []


def test_controller_state_rejects_string_stick():
    """controller_state() takes (x, y) tuples for its sticks, not direction-name strings."""
    sw, fake = make()
    with pytest.raises(ValueError, match="not a string"):
        sw.controller_state("B", left_joystick="up", duration_ms=2000)
    assert fake.commands() == []


def test_controller_state_rejects_mixed_arguments():
    """The buttons argument rejects d-pad directions (and vice versa), pointing at the other argument."""
    sw, fake = make()
    with pytest.raises(ValueError, match="dpad argument"):
        sw.controller_state("ZL+up", duration_ms=60)
    with pytest.raises(ValueError, match="buttons argument"):
        sw.controller_state(None, "B", duration_ms=60)
    assert fake.commands() == []


def test_controller_state_mixed_combination():
    """Buttons and d-pad held via separate arguments go out as a single controller_state with the right bitfield, d-pad position, neutral sticks and duration last."""
    sw, fake = make()
    sw.controller_state("ZL", "up", duration_ms=60)
    assert fake.commands() == ["controller_state"]
    (buttons, dpad, left_x, left_y, right_x, right_y, duration), = fake.arguments("controller_state")
    assert duration == 60
    assert buttons == BUTTON_BITS["ZL"]
    assert dpad == DPAD_POSITIONS["UP"]
    assert (left_x, left_y, right_x, right_y) == (0.0, 0.0, 0.0, 0.0)


def test_push_left_joystick():
    """An (x, y) tuple is forwarded to the backend as separate x, y arguments, with delay = hold + release."""
    sw, fake = make()
    sw.push_left_joystick((0.0, 1.0), hold_ms=1500)
    (x, y, delay, hold, release), = fake.arguments("push_left_joystick")
    assert (x, y) == (0.0, 1.0) and hold == 1500 and delay == 1500 + release


def test_push_left_joystick_rejects_string():
    """push_left_joystick() wants an (x, y) tuple, not a direction-name string."""
    sw, fake = make()
    with pytest.raises(ValueError, match="not a string"):
        sw.push_left_joystick("up")
    with pytest.raises(ValueError):
        sw.push_left_joystick(None)
    assert fake.commands() == []


def test_push_right_joystick():
    """A [x, y] list is accepted too, forwarded with the requested hold and release."""
    sw, fake = make()
    sw.push_right_joystick(joystick_position=[0.5, -0.5], hold_ms=300, release_ms=100)
    (x, y, delay, hold, release), = fake.arguments("push_right_joystick")
    assert (x, y) == (0.5, -0.5) and hold == 300 and release == 100 and delay == 400


def test_controller_state_combination():
    """Buttons and both sticks can be held at once via controller_state, with everything not specified left neutral."""
    sw, fake = make()
    sw.controller_state("B", left_joystick=(0.0, 1.0), right_joystick=(1.0, 0.0), duration_ms=2000)
    (buttons, dpad, left_x, left_y, right_x, right_y, duration), = fake.arguments("controller_state")
    assert duration == 2000
    assert buttons == BUTTON_BITS["B"]
    assert dpad == DPAD_NONE
    assert (left_x, left_y) == (0.0, 1.0)
    assert (right_x, right_y) == (1.0, 0.0)


def test_wait():
    """wait() queues a backend wait of the requested length; a zero-length wait is a no-op."""
    sw, fake = make()
    sw.wait(1000)
    assert fake.arguments("wait") == [(1000,)]
    sw.wait(0)  # a zero-length wait is a no-op
    assert fake.arguments("wait") == [(1000,)]


def test_validation():
    """Negative durations, unknown buttons and contradictory d-pad directions all raise ValueError without queueing anything."""
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_button("A", hold_ms=-1)
    with pytest.raises(ValueError):
        sw.push_button("Q")
    with pytest.raises(ValueError):
        sw.push_dpad("up+down")
    assert fake.commands() == []


def test_wait_for_all_requests_and_cancel():
    """wait_for_all_requests() forwards to the backend, and cancel_all_and_wait() reports the device's acknowledgement (True, then False when the fake stops confirming) on each call."""
    sw, fake = make()
    sw.wait_for_all_requests()
    assert sw.cancel_all_and_wait() is True
    assert fake.cancel_count == 1
    fake.confirm_cancel = False
    assert sw.cancel_all_and_wait() is False
    assert fake.cancel_count == 2


def test_close():
    """close() cancels the queued commands, makes subsequent commands raise, and is idempotent."""
    sw, fake = make()
    sw.close()
    assert fake.cancel_count == 1  # close() stops the device first
    with pytest.raises(RuntimeError, match="closed"):
        sw.push_button("A")
    with pytest.raises(RuntimeError, match="closed"):
        sw.wait_for_all_requests()
    sw.close()  # idempotent
    assert fake.cancel_count == 1


def test_context_manager():
    """The context manager closes the controller on exit; commands after that raise."""
    fake = FakeController()
    with SwitchController(backend=fake) as sw:
        sw.push_button("A")
    with pytest.raises(RuntimeError, match="closed"):
        sw.push_button("A")
