/*  Device Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DeviceRoutines_H
#define PokemonAutomation_DeviceRoutines_H

#include "MessageProtocol.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace Microcontroller{

void seqnum_reset(BotBase& botbase);
uint32_t request_stop(BotBase& botbase);

uint32_t protocol_version(const BotBaseContext& context);
uint32_t program_version(const BotBaseContext& context);
uint8_t program_id(const BotBaseContext& context);


class DeviceRequest_seqnum_reset : public BotBaseRequest{
public:
    pabb_MsgInfoSeqnumReset params;
    DeviceRequest_seqnum_reset()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_SEQNUM_RESET, params);
    }
};
class DeviceRequest_request_stop : public BotBaseRequest{
public:
    pabb_MsgRequestProtocolVersion params;
    DeviceRequest_request_stop()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_STOP, params);
    }
};
class DeviceRequest_protocol_version : public BotBaseRequest{
public:
    pabb_MsgRequestProtocolVersion params;
    DeviceRequest_protocol_version()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROTOCOL_VERSION, params);
    }
};
class DeviceRequest_program_version : public BotBaseRequest{
public:
    pabb_MsgRequestProgramVersion params;
    DeviceRequest_program_version()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROGRAM_VERSION, params);
    }
};
class DeviceRequest_program_id : public BotBaseRequest{
public:
    pabb_MsgRequestProgramID params;
    DeviceRequest_program_id()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROGRAM_ID, params);
    }
};



}
}
#endif
