/*  Push Button Framework
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Messages_PushButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_controller_state : public BotBaseRequest{
public:
    pabb_controller_state params;
    DeviceRequest_controller_state(
        uint16_t button,
        uint8_t dpad,
        uint8_t left_joystick_x,
        uint8_t left_joystick_y,
        uint8_t right_joystick_x,
        uint8_t right_joystick_y,
        uint8_t ticks
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.dpad = dpad;
        params.left_joystick_x = left_joystick_x;
        params.left_joystick_y = left_joystick_y;
        params.right_joystick_x = right_joystick_x;
        params.right_joystick_y = right_joystick_y;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_CONTROLLER_STATE, params);
    }
};



}
}
#endif
