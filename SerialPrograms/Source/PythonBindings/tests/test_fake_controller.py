#  SwitchController on top of a FakeController (no hardware, no compiled module).
#  The assertions check the exact backend calls, which mirror the C++ interface of
#  `NintendoSwitch::PybindSwitchProController`
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


def test_press_single_button():
    sw, fake = make()
    sw.press("A")
    assert fake.commands() == ["push_button"]
    (delay, hold, release, bitfield), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["A"]
    assert delay == hold + release and hold == 80 and release == 80


def test_press_combined_and_custom_durations():
    sw, fake = make()
    sw.press("L+R", hold_ms=200, release_ms=50)
    (delay, hold, release, bitfield), = fake.arguments("push_button")
    assert bitfield == BUTTON_BITS["L"] | BUTTON_BITS["R"]
    assert (delay, hold, release) == (250, 200, 50)


def test_press_repeat_dpad():
    sw, fake = make()
    sw.press("up", repeat=3, hold_ms=100)
    args = fake.arguments("push_dpad")
    assert len(args) == 3
    for delay, hold, release, position in args:
        assert position == DPAD_POSITIONS["UP"] and hold == 100


def test_press_mixed_uses_controller_state():
    sw, fake = make()
    sw.press("ZL+up", hold_ms=60, release_ms=30)
    assert fake.commands() == ["controller_state"]
    (duration, buttons, dpad, left_x, left_y, right_x, right_y), = fake.arguments("controller_state")
    assert duration == 60
    assert buttons == BUTTON_BITS["ZL"]
    assert dpad == DPAD_POSITIONS["UP"]
    assert (left_x, left_y, right_x, right_y) == (0.0, 0.0, 0.0, 0.0)
    assert fake.arguments("wait") == [(30,)]


def test_press_needs_something():
    sw, fake = make()
    with pytest.raises(ValueError):
        sw.press(None)
    assert fake.commands() == []


def test_stick():
    sw, fake = make()
    sw.stick("left", "up", duration_ms=1500)
    (delay, hold, release, x, y), = fake.arguments("push_left_joystick")
    assert (x, y) == (0.0, 1.0) and hold == 1500 and delay == 1500 + release
    sw.stick("right", [0.5, -0.5], duration_ms=300, release_ms=100)
    (delay, hold, release, x, y), = fake.arguments("push_right_joystick")
    assert (x, y) == (0.5, -0.5) and delay == 400
    with pytest.raises(ValueError):
        sw.stick("middle", "up")


def test_hold_combination():
    sw, fake = make()
    sw.hold("B", left="up", right=[1.0, 0.0], duration_ms=2000)
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
        sw.press("A", hold_ms=-1)
    with pytest.raises(ValueError):
        sw.press("A", repeat=0)
    with pytest.raises(ValueError):
        sw.press("Q")
    with pytest.raises(ValueError):
        sw.press("up+down")
    assert fake.commands() == []


def test_flush_and_cancel():
    sw, fake = make()
    sw.flush()
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
        sw.press("A")
    with pytest.raises(RuntimeError, match="closed"):
        sw.flush()
    sw.close()  # idempotent
    assert fake.cancel_count == 1


def test_context_manager():
    fake = FakeController()
    with SwitchController(backend=fake) as sw:
        sw.press("A")
    with pytest.raises(RuntimeError, match="closed"):
        sw.press("A")
