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
    // created, or `timeout_millis` passes.
    // Returns true if the controller is ready to receive commands.
    bool wait_for_ready(uint64_t timeout_millis);

    bool is_ready() const;

    // The connection status text shown in the GUI (formatted as HTML), e.g.
    // device name and firmware version, or the error message if the connection failed.
    std::string current_status() const;

    // Block until every command queued so far has been executed by the device.
    // Returns immediately if the controller is not ready.
    void wait_for_all_requests();

public:
    //  Commands. If the controller is not ready, these log an error and do nothing.
    //  They block only if the device's command queue is full.
    //
    //  `delay` is how long to wait before the next command may start, `hold` how long
    //  the input is held, and `release` how long it must stay released before the same
    //  button can be used again. `delay = hold + release` runs commands back to back
    //  like `pbf_press_button()`; `delay < hold` overlaps them, e.g. to hold a button
    //  while moving a stick.

    // Send a wait command to the controller. Nothing is pressed during the wait time.
    // duration: wait duration, milliseconds.
    // If the controller is not ready, these log an error and do nothing.
    // The function will block only if the device's command queue is full.
    void wait(uint64_t duration);

    // Send a button press command to the controller. It will press all buttons in
    // `bitfield` simultaneously. D-pad buttons are excluded and should be called via
    // `push_dpad()`.
    // delay: how long to wait before the next command may start.
    // hold: how long the button press is held, milliseconds.
    // release: how long the buttons stay released before the same button can be used again,
    // milliseconds.
    // bitfield: values from `NintendoSwitch::Button`.
    //
    // `delay = hold + release` runs commands back to back like `pbf_press_button()`;
    // `delay < hold` overlaps commands, e.g. to hold a button while moving a stick.
    // If the controller is not ready, these log an error and do nothing.
    // The function will block only if the device's command queue is full.
    void push_button(uint64_t delay, uint64_t hold, uint64_t release, uint32_t bitfield);
    // Send a D-pad press command to the controller. It will press one or two D-pad buttons to
    // indicate a direction to the game.
    // delay: how long to wait before the next command may start.
    // hold: how long the button press is held, milliseconds.
    // release: how long the buttons stay released before the same button can be used again,
    // milliseconds.
    // position: `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
    //
    // `delay = hold + release` runs commands back to back like `pbf_press_button()`;
    // `delay < hold` overlaps commands, e.g. to hold a button while moving a stick.
    // If the controller is not ready, these log an error and do nothing.
    // The function will block only if the device's command queue is full.
    void push_dpad(uint64_t delay, uint64_t hold, uint64_t release, uint8_t position);
    // Send a left joystick push command to the controller.
    // delay: how long to wait before the next command may start.
    // hold: how long the joystick is pushed, milliseconds.
    // release: how long the joystick stay released before the same the joystick can be used again,
    // milliseconds.
    // x, y: in [-1.0, 1.0] with +x = right and +y = up.
    //
    // `delay = hold + release` runs commands back to back like `pbf_press_button()`;
    // `delay < hold` overlaps commands, e.g. to hold a button while moving a stick.
    // If the controller is not ready, these log an error and do nothing.
    // The function will block only if the device's command queue is full.
    void push_left_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);
    // Send a right joystick push command to the controller.
    // delay: how long to wait before the next command may start.
    // hold: how long the joystick is pushed, milliseconds.
    // release: how long the joystick stay released before the same the joystick can be used again,
    // milliseconds.
    // x, y: in [-1.0, 1.0] with +x = right and +y = up.
    //
    // `delay = hold + release` runs commands back to back like `pbf_press_button()`;
    // `delay < hold` overlaps commands, e.g. to hold a button while moving a stick.
    // If the controller is not ready, these log an error and do nothing.
    // The function will block only if the device's command queue is full.
    void push_right_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);

    // Set the entire controller state at once and hold it for `duration`.
    // Everything not specified is released. This is the most direct way to hold
    // arbitrary combinations, e.g. running (B + left stick) while turning the camera.
    // duration: how long to hold the buttons/joysticks, milliseconds.
    // button_bitfield: one or more non-D-pad buttons, values from `NintendoSwitch::Button`.
    // dpad_position: `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
    // left_x, left_y: in [-1.0, 1.0] with +x = right and +y = up.
    // right_x, right_y: in [-1.0, 1.0] with +x = right and +y = up.
    void controller_state(
        uint64_t duration,
        uint32_t button_bitfield,
        uint8_t dpad_position,
        double left_x, double left_y,
        double right_x, double right_y
    );

private:
    void* m_internals;
};



}
}
#endif
