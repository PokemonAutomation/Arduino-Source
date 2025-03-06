/*  Nintendo Switch Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This is the raw (full) controller API that is exposed to programs.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Controller_H
#define PokemonAutomation_NintendoSwitch_Controller_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "NintendoSwitch_ControllerState.h"
#include "Controllers/Controller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProController;
using ProControllerContext = ControllerContext<ProController>;




//
//  This is the generic interface a Switch pro controller.
//
class ProController : public AbstractController{
public:
    using ContextType = ProControllerContext;

    ProController();
    virtual ~ProController();

    //  Must call before destruction begins.
    void stop() noexcept;


public:
    //  Temporary for refactor: Send custom requests for PABotBase's advanced
    //  RPCs.
    virtual void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) = 0;
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) = 0;


public:

    //  Press all the buttons set in the bitfield simultaneously.
    //  This command will wait until all the selected buttons are ready to
    //  ensure that they are all dispatched simultaneously.
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

    //  Dpad
    virtual void issue_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

    //  Joysticks
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
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
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
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

    //  Alternate pressing "button0" and "button1" as quickly as possible.
    //  "button0" will always be pressed first.
    //  Both buttons will be pressed at least once.
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds dutation
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
    //  The intended use-case of this for fast scrolling in the system menu
    //  where all 3 buttons have the same effect and can be used at the same
    //  time.
    //
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;


public:
    //  Keyboard Input

    virtual void keyboard_release_all() override;
    virtual void keyboard_press(const QKeyEvent& event) override;
    virtual void keyboard_release(const QKeyEvent& event) override;


private:
    class KeyboardManager;
    Pimpl<KeyboardManager> m_keyboard_manager;
};









}
}
#endif
