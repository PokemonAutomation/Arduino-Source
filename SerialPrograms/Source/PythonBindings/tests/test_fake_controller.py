#  SwitchController on top of a FakeController (no hardware, no compiled module).
#  The command names match the backend (`_pa_core.Controller`), which mirrors the
#  C++ interface of `NintendoSwitch::PybindSwitchProController`
#  (SerialPrograms/Source/Integrations/PybindSwitchProController.h), bound by
#  SerialPrograms/Source/PythonBindings/PythonBindings_Module.cpp.
import pytest

from pokemon_automation import BUTTON_BITS, DPAD_NONE, DPAD_POSITIONS, FakeController, SwitchController


def make():
    fake = FakeController()
    sw = SwitchController(backend=fake)
    return sw, fake


def test_requires_port_or_backend():
    with pytest.raises(ValueError):
        SwitchController()


def test_not_ready_raises():
    fake = FakeController(ready=False)
    with pytest.raises(RuntimeError, match="not ready"):
        SwitchController(backend=fake, timeout_s=0.1)


def test_status_and_name_match_backend():
    sw, fake = make()
    assert sw.current_status() == fake.current_status()
    assert sw.controller_name() == fake.controller_name()


def test_push_button_single_button():
    sw, fake = make()
    sw.push_button("A")
    assert fake.commands() == ["push_button"]
    (delay, hold, release, bitfield), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["A"]
    assert delay == hold + release and hold == 80 and release == 80


def test_push_button_combined_and_custom_durations():
    sw, fake = make()
    sw.push_button("L+R", hold_ms=200, release_ms=50)
    (delay, hold, release, bitfield), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["L"] | BUTTON_BITS["R"]
    assert (delay, hold, release) == (250, 200, 50)


def test_push_dpad():
    sw, fake = make()
    sw.push_dpad("up", hold_ms=100)
    (delay, hold, release, position), = fake.arguments("push_dpad")
    assert position == DPAD_POSITIONS["UP"] and hold == 100 and delay == 100 + release


def test_push_button_rejects_dpad():
    sw, fake = make()
    with pytest.raises(ValueError, match="push_dpad"):
        sw.push_button("ZL+up")
    with pytest.raises(ValueError, match="push_dpad"):
        sw.push_button("up")
    assert fake.commands() == []


def test_push_dpad_rejects_buttons():
    sw, fake = make()
    with pytest.raises(ValueError, match="push_button"):
        sw.push_dpad("ZL+up")
    with pytest.raises(ValueError, match="push_button"):
        sw.push_dpad("ZL")
    assert fake.commands() == []


def test_push_button_needs_something():
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_button(None)
    assert fake.commands() == []


def test_push_dpad_needs_something():
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_dpad(None)
    assert fake.commands() == []


def test_controller_state_mixed_combination():
    sw, fake = make()
    sw.controller_state(60, buttons="ZL+up")
    assert fake.commands() == ["controller_state"]
    (duration, buttons, dpad, left_x, left_y, right_x, right_y), = fake.arguments("controller_state")
    assert duration == 60
    assert buttons == BUTTON_BITS["ZL"]
    assert dpad == DPAD_POSITIONS["UP"]
    assert (left_x, left_y, right_x, right_y) == (0.0, 0.0, 0.0, 0.0)


def test_push_left_joystick():
    sw, fake = make()
    sw.push_left_joystick("up", duration_ms=1500)
    (delay, hold, release, x, y), = fake.arguments("push_left_joystick")
    assert (x, y) == (0.0, 1.0) and hold == 1500 and delay == 1500 + release


def test_push_right_joystick():
    sw, fake = make()
    sw.push_right_joystick([0.5, -0.5], duration_ms=300, release_ms=100)
    (delay, hold, release, x, y), = fake.arguments("push_right_joystick")
    assert (x, y) == (0.5, -0.5) and delay == 400


def test_controller_state_combination():
    sw, fake = make()
    sw.controller_state(2000, buttons="B", left="up", right=[1.0, 0.0])
    (duration, buttons, dpad, left_x, left_y, right_x, right_y), = fake.arguments("controller_state")
    assert duration == 2000
    assert buttons == BUTTON_BITS["B"]
    assert dpad == DPAD_NONE
    assert (left_x, left_y) == (0.0, 1.0)
    assert (right_x, right_y) == (1.0, 0.0)


def test_wait():
    sw, fake = make()
    sw.wait(1000)
    assert fake.arguments("wait") == [(1000,)]
    sw.wait(0)  # a zero-length wait is a no-op
    assert fake.arguments("wait") == [(1000,)]


def test_validation():
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.push_button("A", hold_ms=-1)
    with pytest.raises(ValueError):
        sw.push_button("Q")
    with pytest.raises(ValueError):
        sw.push_dpad("up+down")
    assert fake.commands() == []


def test_wait_for_all_requests_and_cancel():
    sw, fake = make()
    sw.wait_for_all_requests()
    assert sw.cancel_all_and_wait() is True
    assert fake.cancel_count == 1
    fake.confirm_cancel = False
    assert sw.cancel_all_and_wait() is False
    assert fake.cancel_count == 2


def test_close():
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
    fake = FakeController()
    with SwitchController(backend=fake) as sw:
        sw.push_button("A")
    with pytest.raises(RuntimeError, match="closed"):
        sw.push_button("A")
