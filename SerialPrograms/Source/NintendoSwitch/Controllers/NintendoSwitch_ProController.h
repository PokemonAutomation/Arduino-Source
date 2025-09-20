/*  Nintendo Switch Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the raw (full) controller API that is exposed to programs.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Controller_H
#define PokemonAutomation_NintendoSwitch_Controller_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/Controller.h"
#include "NintendoSwitch_ControllerButtons.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProController;
using ProControllerContext = ControllerContext<ProController>;


constexpr Button VALID_PRO_CONTROLLER_BUTTONS =
    BUTTON_Y |
    BUTTON_B |
    BUTTON_A |
    BUTTON_X |
    BUTTON_L |
    BUTTON_R |
    BUTTON_ZL |
    BUTTON_ZR |
    BUTTON_MINUS |
    BUTTON_PLUS |
    BUTTON_LCLICK |
    BUTTON_RCLICK |
    BUTTON_HOME |
    BUTTON_CAPTURE |
    BUTTON_UP |
    BUTTON_RIGHT |
    BUTTON_DOWN |
    BUTTON_LEFT |
    BUTTON_GR |
    BUTTON_GL |
    BUTTON_C;



//
//  This is the generic interface to a Switch pro controller.
//
class ProController : public AbstractController{
public:
    using ContextType = ProControllerContext;

    ProController(Logger& logger);
    virtual ~ProController();

public:
    static const char NAME[];
    virtual const char* name() override{
        return NAME;
    };
    virtual ControllerClass controller_class() const override{
        return ControllerClass::PRO_CONTROLLER;
    }


protected:
    //  Must call before destruction begins.
    void stop() noexcept;


public:
    //  Standard Commands

    //  Press all the buttons set in the bitfield simultaneously.
    //  This command will wait until all the selected buttons are ready to
    //  ensure that they are all dispatched simultaneously.
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    ) = 0;

    //  Dpad
    virtual void issue_dpad(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    ) = 0;

    //  Joysticks
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) = 0;
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) = 0;

    //  Gyro: Accelerometer (experimental - API subject to change)
    virtual void issue_gyro_accel_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;
    virtual void issue_gyro_accel_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;
    virtual void issue_gyro_accel_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;
    virtual void issue_gyro_rotate_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;
    virtual void issue_gyro_rotate_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;
    virtual void issue_gyro_rotate_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) = 0;

    //
    //  Press all the following buttons/joysticks simultaneously for the
    //  specified duration. No wait is added at the end. Thus you can issue
    //  these back-to-back to simulate buttons being pressed and released
    //  concurrently with other buttons being held down the whole time.
    //
    //  This command will wait until the controller is fully idle (including
    //  cooldowns) before it starts. This ensures that everything is issued
    //  simultaneously. In other words, there is an implied call to
    //  "issue_barrier()" before executing the state.
    //
    //  The sole purpose of this function is for keyboard commands.
    //  For programs, it is easier to use the individual button/joystick
    //  functions above.
    //
    //  If we need to support new Switch controller functionality
    //  (such as Joycon gyro or new stuff in Switch 2), we can simply add
    //  overloads to this and gate them behind features.
    //
    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y
    ) = 0;


public:
    //
    //  High speed Macros
    //
    //  Be mindful when calling these mashing functions on a tick imprecise
    //  controller. You can guarantee that some (most) of them will be dropped.
    //
    //  Even if you are on a tick-precise controller, it is not advised to call
    //  these if you are micromanaging with tick-level granularity. The exact
    //  timing characteristics and button selection is not specified and may be
    //  context and implementation-dependent.
    //

    //  Mash a button as quickly as possible.
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button
    ) = 0;

    //  Alternate pressing "button0" and "button1" as quickly as possible.
    //  "button0" will always be pressed first.
    //  Both buttons will be pressed at least once.
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds dutation,
        Button button0, Button button1
    ) = 0;

    //  In situations where A, ZL, and RL all do the same thing, use all 3 of
    //  them to logically mash A much faster than is possible with just one
    //  button.
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) = 0;

    //
    //  Send a scroll command in the specified direction.
    //
    //  This will use either the dpad or either joystick - whichever is
    //  available first in the pipeline.
    //
    //  The intended use-case of this is for fast scrolling in the system menu
    //  where all 3 buttons have the same effect and can be used at the same
    //  time.
    //
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    ) = 0;


public:
    //  Keyboard Input

    virtual void keyboard_release_all() override;
    virtual void keyboard_press(const QKeyEvent& event) override;
    virtual void keyboard_release(const QKeyEvent& event) override;

    virtual void add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener) override;
    virtual void remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener) override;

private:
    class KeyboardManager;
    Pimpl<KeyboardManager> m_keyboard_manager;
};









}
}
#endif
