/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Integrations_PybindSwitchController_H
#define PokemonAutomation_Integrations_PybindSwitchController_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{



// Models a Nintendo Switch wired controller connected via serial port. This class is used
// as an interface for Python API and AI agent use.
//
// Commands are queued: every `push_*()` call returns as soon as the command is
// scheduled on the device, not when it finishes. Call `wait_for_all_requests()` to
// block until everything queued so far has actually been executed. This mirrors how
// `pbf_*()` functions work inside the main program.
//
// The wired controller type (Switch 1/2, Pro vs. 3rd-party controller) is
// whatever the device firmware is currently set to. Use the main GUI program once to
// choose it; the device remembers the setting.
// TODO: we should make this PybindSwitchProController interface able to switch
//   controller type in future.
//
// Button bitfields use `NintendoSwitch::Button` values, and d-pad positions use
// `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
// Joystick coordinates are in [-1.0, 1.0] with +x = right and +y = up.
//
// Thread safety: all methods can be called from any thread. In particular
// `cancel_all_and_wait()` may be called while another thread is blocked in
// `wait_for_all_requests()`, e.g. to stop everything in an emergency.
class PybindSwitchProController{
    PybindSwitchProController(const PybindSwitchProController&) = delete;
    void operator=(const PybindSwitchProController&) = delete;

public:
    // Open a connection to the PABotBase2 device on serial port `port_name`.
    // `port_name` may be a full path ("/dev/cu.usbserial-0001") or a bare device name
    // "cu.usbserial-0001" or "COM3".
    // The connection is established asynchronously; After the constructor, call
    // `wait_for_ready()` next to wait until it is ready.
    // Log lines go to the global logger (`global_logger_raw()`) with tag "Pybind",
    // never to stdout.
    PybindSwitchProController(const std::string& port_name);
    ~PybindSwitchProController();

    // Block until the device handshake finishes and a Switch controller has been
    // created, or `timeout_ms` passes.
    // Returns true if the controller is ready to receive commands.
    bool wait_for_ready(uint64_t timeout_ms);

    bool is_ready() const;

    // The connection status text: device name and firmware version,
    // or the error message if the connection failed.
    std::string current_status() const;

    // Name of the active controller implementation, e.g. "Nintendo Switch: Pro Controller".
    // Empty if not ready.
    std::string controller_name() const;

    // Block until every command queued so far has been executed by the device.
    // Returns immediately if the controller is not ready.
    void wait_for_all_requests();

    // Drop every queued command that has not executed yet, return to the neutral
    // state (no buttons pressed, sticks centered), and wait at most `timeout_ms`
    // for the device to acknowledge that it received the cancel.
    //
    // Returns true if acknowledged, false on timeout or if the controller is not ready.
    // Commands pushed from other threads while this is running may land after the
    // cancel and are not cancelled.
    bool cancel_all_and_wait(uint64_t timeout_ms);

public:
    //  Commands. These line up with the pbf functions in
    //  SerialPrograms/Source/NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h

    //  All times are in milliseconds.
    //  `delay_ms`: how long to wait before the next command may start.
    //  `delay = hold + release` runs commands back to back like `pbf_press_button()`;
    //  `delay < hold` overlaps them, e.g. to hold a button while moving a stick.
    //  `hold_ms`: how long the input is held.
    //  `release_ms`: how long it stays released before the same input can be used again.
    //  `duration_ms`: is the total hold time.
    //
    //  These throw InvalidConnectionStateException if the controller is not
    //  ready, and block only if the device's command queue is full.

    //  Similar to `pbf_wait()`: send a wait command to the controller. Nothing is
    //  pressed during the wait time.
    //  duration_ms: wait duration, milliseconds.
    void wait(uint64_t duration_ms);

    //  Similar to `pbf_press_button()`: send a button press command to the
    //  controller. It will press all buttons in `button_bitfield` simultaneously.
    //  D-pad buttons are excluded and should be called via `push_dpad()`.
    //  button_bitfield: values from `NintendoSwitch::Button`.
    void push_button(uint32_t button_bitfield, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms);

    //  Similar to `pbf_press_dpad()`: send a D-pad press command to the
    //  controller. It will press one or two D-pad buttons to indicate a direction
    //  to the game.
    //  dpad_position: `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
    void push_dpad(uint8_t dpad_position, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms);

    //  Similar to `pbf_move_left_joystick()`: move the left joystick towards a
    //  2D direction.
    //  x, y: in [-1.0, 1.0] with +x = right and +y = up.
    void push_left_joystick(double x, double y, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms);

    //  Similar to `pbf_move_right_joystick()`: move the right joystick towards a
    //  2D direction.
    //  x, y: in [-1.0, 1.0] with +x = right and +y = up.
    void push_right_joystick(double x, double y, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms);

    //  Similar to `pbf_controller_state()`: set the entire controller state at
    //  once and hold it for `duration_ms`. Everything not specified is released.
    //  This is the most direct way to hold arbitrary combinations, e.g. running
    //  (B + left stick) while turning the camera.
    //  button_bitfield: one or more non-D-pad buttons, values from `NintendoSwitch::Button`.
    //  dpad_position: `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
    //  left_x, left_y: in [-1.0, 1.0] with +x = right and +y = up.
    //  right_x, right_y: in [-1.0, 1.0] with +x = right and +y = up.
    //  duration_ms: how long to hold the buttons/d-pad/joysticks, in milliseconds.
    void controller_state(
        uint32_t button_bitfield,
        uint8_t dpad_position,
        double left_x, double left_y,
        double right_x, double right_y,
        uint64_t duration_ms
    );

private:
    void* m_internals;
};



}
}
#endif
