/*  SerialPABotBase (Nintendo Switch 2 Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_NS2_WiredController_H
#define PokemonAutomation_SerialPABotBase_NS2_WiredController_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS2_WiredController.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


class DeviceRequest_NS2_WiredController_ControllerStateMs : public BotBaseRequest{
public:
    pabb_Message_Command_NS2_WiredController_State params;
    DeviceRequest_NS2_WiredController_ControllerStateMs(
        uint16_t milliseconds,
        uint16_t buttons,
        uint8_t dpad_byte,
        uint8_t left_joystick_x,
        uint8_t left_joystick_y,
        uint8_t right_joystick_x,
        uint8_t right_joystick_y
    )
        : BotBaseRequest(true)
    {
        memset(&params, 0, sizeof(params));

        params.seqnum = 0;
        params.milliseconds = milliseconds;

        params.report.buttons0 = (uint8_t)buttons;
        params.report.buttons1 = (uint8_t)(buttons >> 8);
        params.report.dpad_byte = dpad_byte;
        params.report.left_joystick_x = left_joystick_x;
        params.report.left_joystick_y = left_joystick_y;
        params.report.right_joystick_x = right_joystick_x;
        params.report.right_joystick_y = right_joystick_y;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS2_WIRED_CONTROLLER_STATE, params);
    }
};



}
}
#endif
