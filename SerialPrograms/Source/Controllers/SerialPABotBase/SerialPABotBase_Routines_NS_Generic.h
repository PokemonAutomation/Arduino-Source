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



class DeviceRequest_NS_Generic_ControllerStateTicks : public BotBaseRequest{
public:
    pabb_Message_NS_Generic_ControllerStateTicks params;
    DeviceRequest_NS_Generic_ControllerStateTicks(
        uint16_t buttons,
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
        params.buttons = buttons;
        params.dpad = dpad;
        params.left_joystick_x = left_joystick_x;
        params.left_joystick_y = left_joystick_y;
        params.right_joystick_x = right_joystick_x;
        params.right_joystick_y = right_joystick_y;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_NS_GENERIC_CONTROLLER_STATE_TICKS, params);
    }
};



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
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.buttons = buttons;
        params.dpad = dpad;
        params.left_joystick_x = left_joystick_x;
        params.left_joystick_y = left_joystick_y;
        params.right_joystick_x = right_joystick_x;
        params.right_joystick_y = right_joystick_y;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_NS_GENERIC_CONTROLLER_STATE_MS, params);
    }
};



}
}
#endif
