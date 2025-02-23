/*  Device Functions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_Device_H
#define PokemonAutomation_NintendoSwitch_Messages_Device_H

#include "Common/Microcontroller/MessageProtocol.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_system_clock : public BotBaseRequest{
public:
    pabb_system_clock params;
    DeviceRequest_system_clock()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_CLOCK, params);
    }
};
#if 0
class DeviceRequest_set_leds : public BotBaseRequest{
public:
    pabb_MsgCommandSetLeds params;
    DeviceRequest_set_leds(bool on)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.on = on;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SET_LED_STATE, params);
    }
};
class DeviceRequest_end_program_callback : public BotBaseRequest{
public:
    pabb_end_program_callback params;
    DeviceRequest_end_program_callback()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_END_PROGRAM_CALLBACK, params);
    }
};
#endif



}
}
#endif
