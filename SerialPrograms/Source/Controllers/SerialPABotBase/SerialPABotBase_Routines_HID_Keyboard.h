/*  SerialPABotBase (HID Keyboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_HID_Keyboard_H
#define PokemonAutomation_SerialPABotBase_HID_Keyboard_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_HID_Keyboard.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


class DeviceRequest_HID_Keyboard_StateMs : public BotBaseRequest{
public:
    pabb_Message_Command_HID_Keyboard_State params;
    DeviceRequest_HID_Keyboard_StateMs(
        uint16_t milliseconds,
        const pabb_HID_Keyboard_State& report
    )
        : BotBaseRequest(true)
    {
        memset(&params, 0, sizeof(params));

        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.report = report;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_HID_KEYBOARD_STATE, params);
    }
};



}
}
#endif
