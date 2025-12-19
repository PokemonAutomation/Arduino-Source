/*  SerialPABotBase Messages - HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_HID_Keyboard_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_HID_Keyboard_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_HID_Keyboard.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_HID_Keyboard_StateMs : public BotBaseMessageType{
    using Params = pabb_Message_Command_HID_Keyboard_State;
public:
    MessageType_HID_Keyboard_StateMs()
        : BotBaseMessageType(
            "PABB_MSG_COMMAND_HID_KEYBOARD_STATE",
            PABB_MSG_COMMAND_HID_KEYBOARD_STATE,
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

        //  Do not log the contents of the command due to privacy concerns.
        //  (people entering passwords)
#if 0
        ret += ", 0x" + tostr_hex((int)params.report.key[0]);
        ret += ", 0x" + tostr_hex((int)params.report.key[1]);
        ret += ", 0x" + tostr_hex((int)params.report.key[2]);
        ret += ", 0x" + tostr_hex((int)params.report.key[3]);
        ret += ", 0x" + tostr_hex((int)params.report.key[4]);
        ret += ", 0x" + tostr_hex((int)params.report.key[5]);
#endif
        return ret;
    }
};
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
