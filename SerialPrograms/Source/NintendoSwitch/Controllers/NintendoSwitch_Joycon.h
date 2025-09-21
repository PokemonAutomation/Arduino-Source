/*  Nintendo Switch Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Joycon_H
#define PokemonAutomation_NintendoSwitch_Joycon_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "NintendoSwitch_ControllerButtons.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class JoyconController;
using JoyconContext = ControllerContext<JoyconController>;



constexpr Button VALID_LEFT_JOYCON_BUTTONS =
    BUTTON_DOWN |
    BUTTON_UP |
    BUTTON_RIGHT |
    BUTTON_LEFT |
    BUTTON_LEFT_SR |
    BUTTON_LEFT_SL |
    BUTTON_L |
    BUTTON_ZL |
    BUTTON_MINUS |
    BUTTON_LCLICK |
    BUTTON_CAPTURE;

constexpr Button VALID_RIGHT_JOYCON_BUTTONS =
    BUTTON_Y |
    BUTTON_X |
    BUTTON_B |
    BUTTON_A |
    BUTTON_RIGHT_SR |
    BUTTON_RIGHT_SL |
    BUTTON_R |
    BUTTON_ZR |
    BUTTON_PLUS |
    BUTTON_RCLICK |
    BUTTON_HOME |
    BUTTON_C;



class JoyconController : public AbstractController{
public:
    using ContextType = JoyconContext;

    JoyconController(Logger& logger, ControllerType controller_type);
    virtual ~JoyconController();


public:
    static const char NAME[];
    virtual const char* name() override{
        return NAME;
    };


protected:
    //  Must call before destruction begins.
    void stop() noexcept;


public:
    //  Press all the buttons set in the bitfield simultaneously.
    //  This command will wait until all the selected buttons are ready to
    //  ensure that they are all dispatched simultaneously.
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

    virtual void issue_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
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
        Button button,
        uint8_t joystick_x, uint8_t joystick_y,
        Milliseconds hold
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
        Button button, Milliseconds duration
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



class LeftJoycon : public JoyconController{
public:
    using JoyconController::JoyconController;

    static const char NAME[];
    virtual const char* name() override{
        return NAME;
    };
    virtual ControllerClass controller_class() const override{
        return ControllerClass::LEFT_JOYCON;
    }    
};
class RightJoycon : public JoyconController{
public:
    using JoyconController::JoyconController;

    static const char NAME[];
    virtual const char* name() override{
        return NAME;
    };
    virtual ControllerClass controller_class() const override{
        return ControllerClass::RIGHT_JOYCON;
    } 
};




}
}
#endif
