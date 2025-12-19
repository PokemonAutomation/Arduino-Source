/*  SerialPABotBase Messages - NS Wired Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_NS_WiredController_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_NS_WiredController_H

#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS_WiredController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_NS_WiredController_ControllerStateMs : public BotBaseMessageType{
    using Params = pabb_Message_Command_NS_WiredController_State;
public:
    MessageType_NS_WiredController_ControllerStateMs()
        : BotBaseMessageType(
            "PABB_MSG_COMMAND_NS_WIRED_CONTROLLER_STATE",
            PABB_MSG_COMMAND_NS_WIRED_CONTROLLER_STATE,
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
        uint32_t buttons = params.report.buttons0;
        buttons |= (uint32_t)params.report.buttons1 << 8;
        if (params.report.dpad_byte & 0x80){
            buttons |= NintendoSwitch::BUTTON_C;
        }
        ret += ", buttons = " + std::to_string(buttons) + " (" + button_to_string((NintendoSwitch::Button)buttons) + ")";
        ret += ", dpad = " + dpad_to_string((NintendoSwitch::DpadPosition)(params.report.dpad_byte & 0x0f));

        ret += ", LJ = (" + std::to_string((int)params.report.left_joystick_x) + "," + std::to_string((int)params.report.left_joystick_y) + ")";
        ret += ", RJ = (" + std::to_string((int)params.report.right_joystick_x) + "," + std::to_string((int)params.report.right_joystick_y) + ")";
#endif
        return ret;
    }
};
class DeviceRequest_NS_WiredController_ControllerStateMs : public BotBaseRequest{
public:
    pabb_Message_Command_NS_WiredController_State params;
    DeviceRequest_NS_WiredController_ControllerStateMs(
        uint16_t milliseconds,
        uint16_t buttons,
        uint8_t dpad_byte,
        uint8_t left_joystick_x,
        uint8_t left_joystick_y,
        uint8_t right_joystick_x,
        uint8_t right_joystick_y
    )
        : BotBaseRequest(true)
    {
        memset(&params, 0, sizeof(params));

        params.seqnum = 0;
        params.milliseconds = milliseconds;

        params.report.buttons0 = (uint8_t)buttons;
        params.report.buttons1 = (uint8_t)(buttons >> 8);
        params.report.dpad_byte = dpad_byte;
        params.report.left_joystick_x = left_joystick_x;
        params.report.left_joystick_y = left_joystick_y;
        params.report.right_joystick_x = right_joystick_x;
        params.report.right_joystick_y = right_joystick_y;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS_WIRED_CONTROLLER_STATE, params);
    }
};



}
}
#endif
