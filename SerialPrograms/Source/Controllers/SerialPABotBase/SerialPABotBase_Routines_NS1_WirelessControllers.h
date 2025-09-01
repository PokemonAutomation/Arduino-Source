/*  SerialPABotBase (Nintendo Switch 1 Wireless Controllers)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_ESP32_Routines_H
#define PokemonAutomation_SerialPABotBase_ESP32_Routines_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS1_WirelessControllers.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




class MessageControllerReadSpi : public BotBaseRequest{
public:
    pabb_Message_NS1_WirelessController_ReadSpi params;
    MessageControllerReadSpi(ControllerType controller_type, uint32_t address, uint8_t bytes)
        : BotBaseRequest(false)
    {
        uint32_t controller_id = PABB_CID_NONE;
        switch (controller_type){
        case ControllerType::NintendoSwitch_WirelessProController:
            controller_id = PABB_CID_NintendoSwitch_WirelessProController;
            break;
        case ControllerType::NintendoSwitch_LeftJoycon:
            controller_id = PABB_CID_NintendoSwitch_LeftJoycon;
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            controller_id = PABB_CID_NintendoSwitch_RightJoycon;
            break;
        default:;
        }
        params.seqnum = 0;
        params.controller_type = controller_id;
        params.address = address;
        params.bytes = bytes;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_READ_SPI, params);
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
            controller_id = PABB_CID_NintendoSwitch_WirelessProController;
            break;
        case ControllerType::NintendoSwitch_LeftJoycon:
            controller_id = PABB_CID_NintendoSwitch_LeftJoycon;
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            controller_id = PABB_CID_NintendoSwitch_RightJoycon;
            break;
        default:;
        }
        pabb_Message_NS1_WirelessController_WriteSpi params;
        params.seqnum = 0;
        params.controller_type = controller_id;
        params.address = address;
        params.bytes = bytes;
        data = std::string((char*)&params, sizeof(params));
        data += std::string((const char*)p_data, bytes);
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_WRITE_SPI, data);
    }
};
class MessageControllerStateButtons : public BotBaseRequest{
public:
    pabb_Message_Command_NS1_WirelessController_Buttons params;
    MessageControllerStateButtons(
        uint16_t milliseconds,
        const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& state
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.buttons = state;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_BUTTONS, params);
    }
};
class MessageControllerStateFull : public BotBaseRequest{
public:
    pabb_Message_Command_NS1_WirelessController_FullState params;
    MessageControllerStateFull(
        uint16_t milliseconds,
        const pabb_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
        const pabb_NintendoSwitch_WirelessController_State0x30_GyroX3& gyro
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.milliseconds = milliseconds;
        params.state.buttons = buttons;
        params.state.gyro = gyro;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_FULL_STATE, params);
    }
};



}
}
#endif
