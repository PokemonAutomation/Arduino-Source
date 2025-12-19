/*  SerialPABotBase Messages - Base Protocol Controller Mode
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_ControllerMode_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_ControllerMode_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_ReadControllerMode : public BotBaseMessageType{
    using Params = pabb_MsgRequestReadControllerMode;
public:
    MessageType_ReadControllerMode()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_READ_CONTROLLER_MODE",
            PABB_MSG_REQUEST_READ_CONTROLLER_MODE,
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
class DeviceRequest_read_controller_mode : public BotBaseRequest{
public:
    pabb_MsgRequestReadControllerMode params{};
    DeviceRequest_read_controller_mode()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_READ_CONTROLLER_MODE, params);
    }
};



class MessageType_ChangeControllerMode : public BotBaseMessageType{
    using Params = pabb_MsgRequestChangeControllerMode;
public:
    MessageType_ChangeControllerMode()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_CHANGE_CONTROLLER_MODE",
            PABB_MSG_REQUEST_CHANGE_CONTROLLER_MODE,
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
        ret += ", controller = " + tostr_hex(params.controller_id);
        return ret;
    }
};
class DeviceRequest_change_controller_mode : public BotBaseRequest{
public:
    pabb_MsgRequestChangeControllerMode params{};
    DeviceRequest_change_controller_mode(uint32_t controller_id)
        : BotBaseRequest(false)
    {
        params.controller_id = controller_id;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_CHANGE_CONTROLLER_MODE, params);
    }
};



class MessageType_ResetToController : public BotBaseMessageType{
    using Params = pabb_MsgRequestChangeControllerMode;
public:
    MessageType_ResetToController()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_RESET_TO_CONTROLLER",
            PABB_MSG_REQUEST_RESET_TO_CONTROLLER,
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
        ret += ", controller = " + tostr_hex(params.controller_id);
        return ret;
    }
};
class DeviceRequest_reset_to_controller : public BotBaseRequest{
public:
    pabb_MsgRequestChangeControllerMode params{};
    DeviceRequest_reset_to_controller(uint32_t controller_id)
        : BotBaseRequest(false)
    {
        params.controller_id = controller_id;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_RESET_TO_CONTROLLER, params);
    }
};



}
}
#endif
