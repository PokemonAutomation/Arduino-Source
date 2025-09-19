/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerState_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerState_H

#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProControllerState : public ControllerState{
public:
    virtual void clear() override;
    virtual bool operator==(const ControllerState& x) const override;
    virtual bool is_neutral() const override;

    virtual JsonObject serialize_state() const override;

public:
    Button buttons = BUTTON_NONE;
    DpadPosition dpad = DPAD_NONE;
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
};

struct ProControllerDeltas{
    Button buttons = BUTTON_NONE;
    int dpad_x = 0;
    int dpad_y = 0;
    int left_x = 0;
    int left_y = 0;
    int right_x = 0;
    int right_y = 0;

    void operator+=(const ProControllerDeltas& x);

    //  Returns true if neutral.
    bool to_state(ProControllerState& state) const;
};



class JoyconState : public ControllerState{
public:
    virtual void clear() override;
    virtual bool operator==(const ControllerState& x) const override;
    virtual bool is_neutral() const override;

    virtual JsonObject serialize_state() const override;

public:
    Button buttons = BUTTON_NONE;
    uint8_t joystick_x = 128;
    uint8_t joystick_y = 128;
};

struct JoyconDeltas{
    Button buttons = BUTTON_NONE;
    int joystick_x = 0;
    int joystick_y = 0;

    void operator+=(const JoyconDeltas& x);

    //  Returns true if neutral.
    bool to_state(JoyconState& state) const;
};






}
}
#endif
