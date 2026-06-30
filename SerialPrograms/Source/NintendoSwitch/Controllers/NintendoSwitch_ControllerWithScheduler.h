/*  Nintendo Switch Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This implements most of the SwitchController API using only the controller
 *  state function. It uses SuperscalarScheduler to do this.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H
#define PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H

#include "Common/Cpp/CancellableScope.h"
#include "Controllers/Joystick.h"
#include "Controllers/Schedulers/ControllerWithScheduler.h"
#include "NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch_Gyro.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


struct SwitchControllerState{
    Button buttons = BUTTON_NONE;
    DpadPosition dpad = DpadPosition::DPAD_NONE;
    JoystickPosition left_joystick;
    JoystickPosition right_joystick;

    GyroState gyro;
};


enum class SwitchResource{
    BUTTON_NONE,
    BUTTON_Y,
    BUTTON_B,
    BUTTON_A,
    BUTTON_X,
    BUTTON_L,
    BUTTON_R,
    BUTTON_ZL,
    BUTTON_ZR,
    BUTTON_MINUS,
    BUTTON_PLUS,
    BUTTON_LCLICK,
    BUTTON_RCLICK,
    BUTTON_HOME,
    BUTTON_CAPTURE,
    BUTTON_GR,
    BUTTON_GL,
    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_LEFT_SL,
    BUTTON_LEFT_SR,
    BUTTON_RIGHT_SL,
    BUTTON_RIGHT_SR,
    BUTTON_C,

    DPAD,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,

    GYRO,
};

class SwitchCommand : public SchedulerResource{
public:
    using SchedulerResource::SchedulerResource;
    virtual void apply(SwitchControllerState& state) const = 0;
};
class SwitchCommand_Button : public SwitchCommand{
public:
    SwitchCommand_Button(SwitchResource id)
        : SwitchCommand((size_t)id)
    {}
    virtual void apply(SwitchControllerState& state) const{
        state.buttons |= (Button)((ButtonFlagType)1 << id);
    }
};
struct SwitchCommand_Dpad : public SwitchCommand{
    DpadPosition position;

    SwitchCommand_Dpad(DpadPosition position)
        : SwitchCommand((size_t)SwitchResource::DPAD)
        , position(position)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        state.dpad = position;
    }
};
struct SwitchCommand_LeftJoystick : public SwitchCommand{
    JoystickPosition position;

    SwitchCommand_LeftJoystick(const JoystickPosition& position)
        : SwitchCommand((size_t)SwitchResource::JOYSTICK_LEFT)
        , position(position)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        state.left_joystick = position;
    }
};
struct SwitchCommand_RightJoystick : public SwitchCommand{
    JoystickPosition position;

    SwitchCommand_RightJoystick(const JoystickPosition& position)
        : SwitchCommand((size_t)SwitchResource::JOYSTICK_RIGHT)
        , position(position)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        state.right_joystick = position;
    }
};
struct SwitchCommand_GyroState : public SwitchCommand{
    GyroState state;

    SwitchCommand_GyroState(const GyroState& state)
        : SwitchCommand((size_t)SwitchResource::GYRO)
        , state(state)
    {}
    virtual void apply(SwitchControllerState& controller_state) const override{
        controller_state.gyro = state;
    }
};




struct SplitDpad{
    bool up = false;
    bool right = false;
    bool down = false;
    bool left = false;
};
inline SplitDpad convert_unified_to_split_dpad(DpadPosition dpad){
    switch (dpad){
    case DpadPosition::DPAD_UP:
        return {true, false, false, false};
    case DpadPosition::DPAD_UP_RIGHT:
        return {true, true, false, false};
    case DpadPosition::DPAD_RIGHT:
        return {false, true, false, false};
    case DpadPosition::DPAD_DOWN_RIGHT:
        return {false, true, true, false};
    case DpadPosition::DPAD_DOWN:
        return {false, false, true, false};
    case DpadPosition::DPAD_DOWN_LEFT:
        return {false, false, true, true};
    case DpadPosition::DPAD_LEFT:
        return {false, false, false, true};
    case DpadPosition::DPAD_UP_LEFT:
        return {true, false, false, true};
    default:
        return {false, false, false, false};
    }
}





class ControllerWithScheduler : public PokemonAutomation::ControllerWithScheduler{
public:
    using PokemonAutomation::ControllerWithScheduler::ControllerWithScheduler;


public:
    //  Superscalar Commands (the "ssf" framework)

    void issue_buttons(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    );
    void issue_dpad(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    );
    void issue_left_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    );
    void issue_right_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    );

    void issue_gyro_motion(
        Cancellable* cancellable,
        ControllerClass controller_class,
        Milliseconds duration,
        const GyroFunction& function
    );

    void issue_full_controller_state(
        Cancellable* cancellable,
        bool enable_logging,
        Milliseconds hold,
        Button button,
        DpadPosition dpad,
        const JoystickPosition& left_joystick,
        const JoystickPosition& right_joystick
    );


public:
    //  High speed RPCs.

    void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        Button button,
        Milliseconds delay,
        Milliseconds hold,
        Milliseconds cooldown
    );
    void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    );
    void issue_mash_AZs(
        Cancellable* cancellable,
        Milliseconds duration
    );
    void issue_system_scroll(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    );
};




}
}
#endif
