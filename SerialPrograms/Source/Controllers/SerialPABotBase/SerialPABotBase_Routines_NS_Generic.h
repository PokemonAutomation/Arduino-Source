/*  SerialPABotBase (Nintendo Switch Generic Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_NS_Generic_H
#define PokemonAutomation_SerialPABotBase_NS_Generic_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS_Generic.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


class DeviceRequest_NS_Generic_ControllerStateMs : public BotBaseRequest{
public:
    pabb_Message_NS_Generic_ControllerStateMs params;
    DeviceRequest_NS_Generic_ControllerStateMs(
        uint16_t milliseconds,
        uint16_t buttons,
        uint8_t dpad,
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
//        params.report.buttons2 = 0;
//        params.report.buttons3 = 0;

        params.report.dpad = dpad;
        params.report.left_joystick_x = left_joystick_x;
        params.report.left_joystick_y = left_joystick_y;
        params.report.right_joystick_x = right_joystick_x;
        params.report.right_joystick_y = right_joystick_y;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_NS_GENERIC_CONTROLLER_STATE_MS, params);
    }
};



}
}
#endif
