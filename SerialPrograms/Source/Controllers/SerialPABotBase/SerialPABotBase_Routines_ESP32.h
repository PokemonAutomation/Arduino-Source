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




class MessageControllerReadSpi : public BotBaseRequest{
public:
    pabb_Message_ESP32_ReadSpi params;
    MessageControllerReadSpi(ControllerType controller_type, uint32_t address, uint8_t bytes)
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
        params.address = address;
        params.bytes = bytes;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_READ_SPI, params);
    }
};
class MessageControllerWriteSpi : public BotBaseRequest{
public:
    std::string data;
    MessageControllerWriteSpi(
        ControllerType controller_type,
        uint32_t address, uint8_t bytes,
        const void* p_data
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
        pabb_Message_ESP32_WriteSpi params;
        params.seqnum = 0;
        params.controller_type = controller_id;
        params.address = address;
        params.bytes = bytes;
        data = std::string((char*)&params, sizeof(params));
        data += std::string((const char*)p_data, bytes);
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_WRITE_SPI, data);
    }
};
class MessageControllerStateButtons : public BotBaseRequest{
public:
    pabb_Message_ESP32_CommandButtonState params;
    MessageControllerStateButtons(uint16_t milliseconds, const PABB_NintendoSwitch_ButtonState& state)
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
        const PABB_NintendoSwitch_ButtonState& buttons,
        const PABB_NintendoSwitch_GyroStateX3& gyro
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
