/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerState_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerState_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"

namespace PokemonAutomation{
    class ControllerSession;
namespace NintendoSwitch{


class SwitchControllerState : public ControllerState{
public:
    virtual void clear() override;

    virtual bool operator==(const ControllerState& x) const override;

    virtual bool is_neutral() const override;
    virtual bool send_to_controller(ControllerSession& controller) const override;

public:
    Button buttons = BUTTON_NONE;
    DpadPosition dpad = DPAD_NONE;
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
};


struct ControllerDeltas{
    Button buttons = BUTTON_NONE;
    int dpad_x = 0;
    int dpad_y = 0;
    int left_x = 0;
    int left_y = 0;
    int right_x = 0;
    int right_y = 0;

    void operator+=(const ControllerDeltas& x);

    //  Returns true if neutral.
    bool to_state(SwitchControllerState& state) const;
};







}
}
#endif
