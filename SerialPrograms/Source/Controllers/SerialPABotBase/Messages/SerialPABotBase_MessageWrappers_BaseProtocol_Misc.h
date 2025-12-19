/*  SerialPABotBase Messages - Base Protocol Misc.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Misc_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Misc_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_SystemClock : public BotBaseMessageType{
    using Params = pabb_system_clock;
public:
    MessageType_SystemClock()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_CLOCK",
            PABB_MSG_REQUEST_CLOCK,
            sizeof(Params)
        )
    {}
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        return ret;
    }
};
class DeviceRequest_system_clock : public BotBaseRequest{
public:
    pabb_system_clock params{};
    DeviceRequest_system_clock()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_CLOCK, params);
    }
};



class MessageType_ControllerStatus : public BotBaseMessageType{
    using Params = pabb_Message_RequestStatus;
public:
    MessageType_ControllerStatus()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_STATUS",
            PABB_MSG_REQUEST_STATUS,
            sizeof(Params)
        )
    {}
    virtual bool should_print(const std::string& body) const override{
        return GlobalSettings::instance().LOG_EVERYTHING;
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        return ret;
    }
};
class MessageControllerStatus : public BotBaseRequest{
public:
    pabb_Message_RequestStatus params{};
    MessageControllerStatus()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_STATUS, params);
    }
};



class MessageType_ReadMacAddress : public BotBaseMessageType{
    using Params = pabb_MsgRequestReadMacAddress;
public:
    MessageType_ReadMacAddress()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_READ_MAC_ADDRESS",
            PABB_MSG_REQUEST_READ_MAC_ADDRESS,
            sizeof(Params)
        )
    {}
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        return ret;
    }
};
class DeviceRequest_read_mac_address : public BotBaseRequest{
public:
    pabb_MsgRequestReadMacAddress params{};
    DeviceRequest_read_mac_address(uint32_t mode)
        : BotBaseRequest(false)
    {
        params.mode = mode;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_READ_MAC_ADDRESS, params);
    }
};



class MessageType_PairedMacAddress : public BotBaseMessageType{
    using Params = pabb_MsgRequestPairedMacAddress;
public:
    MessageType_PairedMacAddress()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_PAIRED_MAC_ADDRESS",
            PABB_MSG_REQUEST_PAIRED_MAC_ADDRESS,
            sizeof(Params)
        )
    {}
    virtual bool should_print(const std::string& body) const override{
        return GlobalSettings::instance().LOG_EVERYTHING;
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        ret += ", mode = " + std::to_string(params.mode);
        return ret;
    }
};
class DeviceRequest_paired_mac_address : public BotBaseRequest{
public:
    pabb_MsgRequestPairedMacAddress params{};
    DeviceRequest_paired_mac_address(ControllerType controller_type)
        : BotBaseRequest(false)
    {
        params.mode = controller_type_to_id(controller_type);
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PAIRED_MAC_ADDRESS, params);
    }
};



}
}
#endif
