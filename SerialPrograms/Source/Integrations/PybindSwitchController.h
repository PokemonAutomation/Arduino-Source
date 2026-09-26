/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A GUI-free Nintendo Switch controller that talks to a PABotBase2 microcontroller
 *  over a serial port. It is part of CoreLib and uses only plain types in its
 *  interface, so it can be bound to other languages: the `_pa_core` Python module
 *  (Source/PythonBindings/) wraps it, and the Python package and MCP server are built
 *  on top of that. SerialProgramsCommandLine also uses it directly to test its
 *  functionality.
 */

#ifndef PokemonAutomation_Integrations_PybindSwitchController_H
#define PokemonAutomation_Integrations_PybindSwitchController_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{



// A Nintendo Switch controller (Pro Controller or 3rd-party wired controller) driven
// through a PABotBase2 device, e.g. an ESP32 or Pico flashed with PA firmware.
//
// Commands are queued: every `push_*()` call returns as soon as the command is
// scheduled on the device, not when it finishes. Call `wait_for_all_requests()` to
// block until everything queued so far has actually been executed. This mirrors how
// `pbf_*()` functions work inside the main program.
//
// The controller type (Pro Controller vs. wired controller, Switch 1 vs. Switch 2) is
// whatever the device firmware is currently set to. Use the main GUI program once to
// choose it; the device remembers the setting.
// TODO: we should make this PybindSwitchProController interface able to switch
//   controller type in future.
//
// Button bitfields use `NintendoSwitch::Button` values, and d-pad positions use
// `NintendoSwitch::DpadPosition` values (0 = up, clockwise to 7 = up-left, 8 = none).
// Joystick coordinates are in [-1.0, 1.0] with +x = right and +y = up.
//
// All times are in milliseconds.
//
// Thread safety: all methods can be called from any thread. In particular
// `cancel_all()` and `release_all()` may be called while another thread is blocked in
// `wait_for_all_requests()`, which is how the Python layer implements an emergency stop.
class PybindSwitchProController{
    PybindSwitchProController(const PybindSwitchProController&) = delete;
    void operator=(const PybindSwitchProController&) = delete;

public:
    // Open a connection to the PABotBase2 device on serial port `port_name`.
    // `port_name` may be a full path ("/dev/cu.usbserial-0001") or a bare device name
    // ("cu.usbserial-0001", "COM3").
    // The connection is established asynchronously; call `wait_for_ready()` next.
    // Log lines go to the global logger (`global_logger_raw()`) with tag "Pybind",
    // never to stdout.
    PybindSwitchProController(const std::string& port_name);
    ~PybindSwitchProController();

    // Block until the device handshake finishes and a Switch controller has been
    // created, or the connection fails, or `timeout_millis` passes.
    // Returns true if the controller is ready to receive commands.
    bool wait_for_ready(uint64_t timeout_millis);

    bool is_ready() const;

    // The connection status text shown in the GUI (with HTML tags removed), e.g.
    // device name and firmware version, or the error message if the connection failed.
    std::string current_status() const;

    // Name of the active controller implementation, e.g. "Nintendo Switch: Pro Controller".
    // Empty if not ready.
    std::string controller_name() const;

    // Block until every command queued so far has been executed by the device.
    // Returns immediately if the controller is not ready.
    void wait_for_all_requests();

    // Request that the device drop every queued command that has not executed yet
    // and return to the neutral state (no buttons pressed, sticks centered). Safe to
    // call at any time, including from another thread. The controller stays usable.
    //
    // This does not wait: it returns as soon as the cancel request is handed to the
    // serial connection, usually before the device has acted on it. If nothing is
    // queued, no request is sent (the device is already idle). If the serial send
    // buffer is full, the request is sent later, when the next command is sent or a
    // queued command finishes. Use `release_all()` to also wait for confirmation.
    void cancel_all();

    // Same as `cancel_all()`, then wait for the device to confirm that it is in the
    // neutral state, for at most `timeout_millis`.
    //
    // Confirmation (see `release_all_and_confirm()` in Controllers/ControllerRelease.h)
    // works by queueing a short neutral no-op after the cancel and
    // waiting for the device to report that the no-op finished. The serial protocol
    // delivers messages in order, so that report means the device has processed the
    // cancel and is holding the neutral state.
    //
    // Returns true if confirmed, false on timeout or if the controller is not ready.
    // The timeout covers waiting for the device; if another thread is in the middle
    // of issuing a command on this controller, that call is allowed to finish first.
    bool release_all(uint64_t timeout_millis);


public:
    //  Commands. These throw InvalidConnectionStateException if the controller is not
    //  ready, and block only if the device's command queue is full.
    //
    //  `delay` is how long to wait before the next command may start, `hold` how long
    //  the input is held, and `release` how long it must stay released before the same
    //  button can be used again. `delay = hold + release` runs commands back to back
    //  like `pbf_press_button()`; `delay < hold` overlaps them, e.g. to hold a button
    //  while moving a stick.

    // Queue a wait. Nothing is pressed during this time.
    void wait(uint64_t duration);

    // Press all buttons in `bitfield` simultaneously.
    void push_button(uint64_t delay, uint64_t hold, uint64_t release, uint32_t bitfield);
    void push_dpad(uint64_t delay, uint64_t hold, uint64_t release, uint8_t position);
    void push_left_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);
    void push_right_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y);

    // Set the entire controller state at once and hold it for `duration`.
    // Everything not specified is released. This is the most direct way to hold
    // arbitrary combinations, e.g. running (B + left stick) while turning the camera.
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
