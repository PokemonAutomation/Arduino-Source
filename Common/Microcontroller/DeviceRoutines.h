/*  Device Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DeviceRoutines_H
#define PokemonAutomation_DeviceRoutines_H

#include "MessageProtocol.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace Microcontroller{

uint32_t protocol_version(BotBaseController& botbase);
uint32_t program_version(BotBaseController& botbase);
uint8_t device_queue_size(BotBaseController& botbase);
uint8_t program_id(BotBaseController& botbase);


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
class DeviceRequest_next_command_interrupt : public BotBaseRequest{
public:
    pabb_MsgRequestNextCmdInterrupt params;
    DeviceRequest_next_command_interrupt()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT, params);
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
class DeviceRequest_queue_size : public BotBaseRequest{
public:
    pabb_MsgRequestProgramVersion params;
    DeviceRequest_queue_size()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_QUEUE_SIZE, params);
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
