/*  SerialPABotBase Routines (Protocol)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Routines_Protocol_H
#define PokemonAutomation_SerialPABotBase_Routines_Protocol_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{

uint32_t protocol_version(BotBaseController& botbase);
uint32_t program_version(BotBaseController& botbase);
uint8_t device_queue_size(BotBaseController& botbase);
uint8_t program_id(BotBaseController& botbase);
uint32_t read_controller_mode(BotBaseController& botbase);
uint32_t change_controller_mode(BotBaseController& botbase, uint32_t mode);


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
class DeviceRequest_read_controller_mode : public BotBaseRequest{
public:
    pabb_MsgRequestReadControllerMode params;
    DeviceRequest_read_controller_mode()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_READ_CONTROLLER_MODE, params);
    }
};
class DeviceRequest_change_controller_mode : public BotBaseRequest{
public:
    pabb_MsgRequestChangeControllerMode params;
    DeviceRequest_change_controller_mode(uint32_t mode)
        : BotBaseRequest(false)
    {
        params.mode = mode;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_CHANGE_CONTROLLER_MODE, params);
    }
};
class MessageControllerStatus : public BotBaseRequest{
public:
    pabb_Message_RequestStatus params;
    MessageControllerStatus()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_STATUS, params);
    }
};
class DeviceRequest_read_mac_address : public BotBaseRequest{
public:
    pabb_MsgRequestReadMacAddress params;
    DeviceRequest_read_mac_address(uint32_t mode)
        : BotBaseRequest(false)
    {
        params.mode = mode;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_READ_MAC_ADDRESS, params);
    }
};



}
}
#endif
