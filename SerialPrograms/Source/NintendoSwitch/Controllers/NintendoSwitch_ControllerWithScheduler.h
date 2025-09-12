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
#include "Controllers/Schedulers/ControllerWithScheduler.h"
#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct SwitchControllerState{
    Button buttons = BUTTON_NONE;
    DpadPosition dpad = DpadPosition::DPAD_NONE;
    uint8_t left_stick_x = 128;
    uint8_t left_stick_y = 128;
    uint8_t right_stick_x = 128;
    uint8_t right_stick_y = 128;

    uint16_t gyro[6];
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

    GYRO_ACCEL_X,
    GYRO_ACCEL_Y,
    GYRO_ACCEL_Z,
    GYRO_ROTATE_X,
    GYRO_ROTATE_Y,
    GYRO_ROTATE_Z,
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
    uint8_t x;
    uint8_t y;

    SwitchCommand_LeftJoystick(uint8_t x, uint8_t y)
        : SwitchCommand((size_t)SwitchResource::JOYSTICK_LEFT)
        , x(x), y(y)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        state.left_stick_x = x;
        state.left_stick_y = y;
    }
};
struct SwitchCommand_RightJoystick : public SwitchCommand{
    uint8_t x;
    uint8_t y;

    SwitchCommand_RightJoystick(uint8_t x, uint8_t y)
        : SwitchCommand((size_t)SwitchResource::JOYSTICK_RIGHT)
        , x(x), y(y)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        state.right_stick_x = x;
        state.right_stick_y = y;
    }
};
struct SwitchCommand_Gyro : public SwitchCommand{
    int16_t value;

    SwitchCommand_Gyro(SwitchResource id, int16_t value)
        : SwitchCommand((size_t)id)
        , value(value)
    {}
    virtual void apply(SwitchControllerState& state) const override{
        size_t index = (ButtonFlagType)id - (ButtonFlagType)SwitchResource::GYRO_ACCEL_X;
        state.gyro[index] = value;
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
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    );
    void issue_dpad(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    );
    void issue_left_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    );
    void issue_right_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    );

    void issue_gyro(
        const Cancellable* cancellable,
        SwitchResource id, const char* name,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    );
    void issue_gyro_accel_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ACCEL_X, "issue_gyro_accel_x", delay, hold, cooldown, value);
    }
    void issue_gyro_accel_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ACCEL_Y, "issue_gyro_accel_y", delay, hold, cooldown, value);
    }
    void issue_gyro_accel_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ACCEL_Z, "issue_gyro_accel_z", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ROTATE_X, "issue_gyro_rotate_x", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ROTATE_Y, "issue_gyro_rotate_y", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, SwitchResource::GYRO_ROTATE_Z, "issue_gyro_rotate_z", delay, hold, cooldown, value);
    }

    void issue_full_controller_state(
        const Cancellable* cancellable,
        Milliseconds hold,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y
    );


public:
    //  High speed RPCs.

    void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button
    );
    void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    );
    void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    );
    void issue_system_scroll(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    );
};




}
}
#endif
