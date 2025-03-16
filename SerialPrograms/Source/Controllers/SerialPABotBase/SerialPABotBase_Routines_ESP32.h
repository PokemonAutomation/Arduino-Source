/*  SerialPABotBase ESP32 Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_ESP32_Routines_H
#define PokemonAutomation_SerialPABotBase_ESP32_Routines_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "Common/SerialPABotBase/SerialPABotBase_Messages_ESP32.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




class MessageControllerStatus : public BotBaseRequest{
public:
    pabb_Message_ESP32_RequestStatus params;
    MessageControllerStatus()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_STATUS, params);
    }
};
class MessageControllerGetColors : public BotBaseRequest{
public:
    pabb_Message_ESP32_GetColors params;
    MessageControllerGetColors(ControllerType controller_type)
        : BotBaseRequest(false)
    {
        uint32_t controller_id = PABB_CID_NONE;
        switch (controller_type){
        case ControllerType::NintendoSwitch_WirelessProController:
            controller_id = PABB_CID_NINTENDO_SWITCH_WIRELESS_PRO_CONTROLLER;
            break;
        case ControllerType::NintendoSwitch_LeftJoycon:
            controller_id = PABB_CID_NINTENDO_SWITCH_LEFT_JOYCON;
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            controller_id = PABB_CID_NINTENDO_SWITCH_RIGHT_JOYCON;
            break;
        default:;
        }
        params.seqnum = 0;
        params.controller_type = controller_id;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_GET_COLORS, params);
    }
};
class MessageControllerSetColors : public BotBaseRequest{
public:
    pabb_Message_ESP32_SetColors params;
    MessageControllerSetColors(
        ControllerType controller_type,
        const NintendoSwitch_ControllerColors& colors
    )
        : BotBaseRequest(false)
    {
        uint32_t controller_id = PABB_CID_NONE;
        switch (controller_type){
        case ControllerType::NintendoSwitch_WirelessProController:
            controller_id = PABB_CID_NINTENDO_SWITCH_WIRELESS_PRO_CONTROLLER;
            break;
        case ControllerType::NintendoSwitch_LeftJoycon:
            controller_id = PABB_CID_NINTENDO_SWITCH_LEFT_JOYCON;
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            controller_id = PABB_CID_NINTENDO_SWITCH_RIGHT_JOYCON;
            break;
        default:;
        }
        params.seqnum = 0;
        params.controller_type = controller_id;
        params.colors = colors;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_SET_COLORS, params);
    }
};
class MessageControllerStateButtons : public BotBaseRequest{
public:
    pabb_Message_ESP32_CommandButtonState params;
    MessageControllerStateButtons(uint16_t milliseconds, const NintendoSwitch_ButtonState& state)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.buttons = state;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_CONTROLLER_STATE_BUTTONS, params);
    }
};
class MessageControllerStateFull : public BotBaseRequest{
public:
    pabb_Message_ESP32_CommandFullState params;
    MessageControllerStateFull(
        uint16_t milliseconds,
        const NintendoSwitch_ButtonState& buttons,
        const NintendoSwitch_GyroState& gyro
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.buttons = buttons;
        params.gyro = gyro;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_CONTROLLER_STATE_FULL, params);
    }
};



}
}
#endif
