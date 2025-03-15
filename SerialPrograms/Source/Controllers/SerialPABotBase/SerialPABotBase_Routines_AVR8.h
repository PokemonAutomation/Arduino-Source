/*  SerialPABotBase AVR8 Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_AVR8_Routines_H
#define PokemonAutomation_SerialPABotBase_AVR8_Routines_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_AVR8.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



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
