/*  SerialPABotBase Messages - NintendoSwitch OEM Controllers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_NintendoSwitch_OemControllers_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_NintendoSwitch_OemControllers_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS1_OemControllers.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
//#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class MessageType_NS1_PlayerLights : public BotBaseMessageType{
    using Params = pabb_MsgInfo_NS1_OemController_PlayerLights;
public:
    MessageType_NS1_PlayerLights()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_PLAYER_LIGHTS",
            PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_PLAYER_LIGHTS,
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
class DeviceRequest_PlayerLights : public BotBaseRequest{
public:
    pabb_MsgInfo_NS1_OemController_PlayerLights params;
    DeviceRequest_PlayerLights(ControllerType controller_type)
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
        params.controller_type = SerialPABotBase::controller_type_to_id(controller_type);
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_PLAYER_LIGHTS, params);
    }
};



class MessageType_NS1_ReadSpi : public BotBaseMessageType{
    using Params = pabb_Message_NS1_OemController_ReadSpi;
public:
    MessageType_NS1_ReadSpi()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_READ_SPI",
            PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_READ_SPI,
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
        ret += ", controller = " + std::to_string(params.controller_type);
        ret += ", address = 0x" + tostr_hex(params.address);
        ret += ", bytes = " + std::to_string((size_t)params.bytes);
        return ret;
    }
};
class DeviceRequest_NS1_ReadSpi : public BotBaseRequest{
public:
    pabb_Message_NS1_OemController_ReadSpi params;
    DeviceRequest_NS1_ReadSpi(ControllerType controller_type, uint32_t address, uint8_t bytes)
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
        params.controller_type = SerialPABotBase::controller_type_to_id(controller_type);
        params.address = address;
        params.bytes = bytes;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_READ_SPI, params);
    }
};



class MessageType_NS1_WriteSpi : public BotBaseMessageType{
    using Params = pabb_Message_NS1_OemController_WriteSpi;
public:
    MessageType_NS1_WriteSpi()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_WRITE_SPI",
            PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_WRITE_SPI,
            0
        )
    {}
    virtual bool is_valid(const std::string& body) const override{
        return body.size() >= sizeof(Params);
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        ret += ", controller = " + std::to_string(params.controller_type);
        ret += ", address = 0x" + tostr_hex(params.address);
        ret += ", bytes = " + std::to_string((size_t)params.bytes);
        return ret;
    }
};
class DeviceRequest_NS1_WriteSpi : public BotBaseRequest{
public:
    std::string data;
    DeviceRequest_NS1_WriteSpi(
        ControllerType controller_type,
        uint32_t address, uint8_t bytes,
        const void* p_data
    )
        : BotBaseRequest(false)
    {
        pabb_Message_NS1_OemController_WriteSpi params;
        params.seqnum = 0;
        params.controller_type = SerialPABotBase::controller_type_to_id(controller_type);
        params.address = address;
        params.bytes = bytes;
        data = std::string((char*)&params, sizeof(params));
        data += std::string((const char*)p_data, bytes);
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_WRITE_SPI, data);
    }
};



class MessageType_NS1_OemControllerStateButtons : public BotBaseMessageType{
    using Params = pabb_Message_Command_NS1_OemController_Buttons;
public:
    MessageType_NS1_OemControllerStateButtons()
        : BotBaseMessageType(
            "PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_BUTTONS",
            PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_BUTTONS,
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
        ret += ", ms = " + std::to_string(params.milliseconds);
        return ret;
    }
};
class DeviceRequest_ControllerStateButtons : public BotBaseRequest{
public:
    pabb_Message_Command_NS1_OemController_Buttons params;
    DeviceRequest_ControllerStateButtons(
        uint16_t milliseconds,
        const pabb_NintendoSwitch_OemController_State0x30_Buttons& state
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.buttons = state;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_BUTTONS, params);
    }
};



class MessageType_NS1_OemControllerStateFull : public BotBaseMessageType{
    using Params = pabb_Message_Command_NS1_OemController_FullState;
public:
    MessageType_NS1_OemControllerStateFull()
        : BotBaseMessageType(
            "PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_FULL_STATE",
            PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_FULL_STATE,
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
        ret += ", ms = " + std::to_string(params.milliseconds);
        return ret;
    }
};
class DeviceRequest_ControllerStateFull : public BotBaseRequest{
public:
    pabb_Message_Command_NS1_OemController_FullState params;
    DeviceRequest_ControllerStateFull(
        uint16_t milliseconds,
        const pabb_NintendoSwitch_OemController_State0x30_Buttons& buttons,
        const pabb_NintendoSwitch_OemController_State0x30_GyroX3& gyro
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.state.buttons = buttons;
        params.state.gyro = gyro;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_FULL_STATE, params);
    }
};



class MessageType_NS1_OemControllerRumble : public BotBaseMessageType{
    using Params = pabb_MsgInfo_NS1_OemController_Rumble;
public:
    MessageType_NS1_OemControllerRumble()
        : BotBaseMessageType(
            "PABB_MSG_INFO_NS1_OEM_CONTROLLER_RUMBLE",
            PABB_MSG_INFO_NS1_OEM_CONTROLLER_RUMBLE,
            sizeof(Params)
        )
    {}
    virtual bool should_print(const std::string& body) const override{
        if (!is_valid(body) || GlobalSettings::instance().LOG_EVERYTHING){
            return true;
        }

        uint32_t left, right;
        memcpy(&left, body.data(), sizeof(uint32_t));
        memcpy(&right, (uint32_t*)body.data() + 1, sizeof(uint32_t));

        const uint32_t NEUTRAL = 0x40400100;
        if (left != 0 && left != NEUTRAL){
            return true;
        }
        if (right != 0 && right != NEUTRAL){
            return true;
        }
        return false;
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        static const char HEX_DIGITS[] = "0123456789abcdef";
        for (size_t c = 0; c < body.size(); c++){
            uint8_t byte = body[c];
            ret += " ";
            ret += HEX_DIGITS[(byte >> 4)];
            ret += HEX_DIGITS[byte & 15];
        }
        return ret;
    }
};
class MessageType_NS1_OemControllerUsbDisallowed : public BotBaseMessageType{
public:
    MessageType_NS1_OemControllerUsbDisallowed()
        : BotBaseMessageType(
            "PABB_MSG_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED",
            PABB_MSG_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED,
            0
        )
    {}
};





}
}
#endif
