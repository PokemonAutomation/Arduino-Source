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
class MessageControllerState : public BotBaseRequest{
public:
    pabb_esp32_report30 params;
    MessageControllerState(uint8_t ticks, bool active, NintendoSwitch_ESP32Report0x30 report)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
        params.active = active;
        params.report = report;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REPORT, params);
    }
};



}
}
#endif
