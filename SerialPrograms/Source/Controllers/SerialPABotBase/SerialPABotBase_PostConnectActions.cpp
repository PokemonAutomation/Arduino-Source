/*  Serial Port (PABotBase) Post-Connect Actinos
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "ClientSource/Connection/PABotBase.h"
#include "Controllers/ControllerTypeStrings.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "SerialPABotBase_Routines_Protocol.h"
#include "SerialPABotBase_Routines_ESP32.h"
#include "SerialPABotBase.h"
#include "SerialPABotBase_PostConnectActions.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{



void run_post_connect_actions_ESP32S3(
    ControllerModeStatus& status,
    const std::string& device_name,
    PABotBase& botbase,
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
    if (!change_controller){
        return;
    }

    Logger& logger = botbase.logger();

    ControllerType desired_controller = change_controller.value();
    switch (desired_controller){
    case ControllerType::NintendoSwitch_WiredController:
    case ControllerType::NintendoSwitch2_WiredController:
    default:;
    }

    uint32_t native_controller_id = controller_type_to_id(desired_controller);
    if (clear_settings){
        botbase.issue_request_and_wait(
            DeviceRequest_reset_to_controller(native_controller_id),
            nullptr
        );
    }else{
        botbase.issue_request_and_wait(
            DeviceRequest_change_controller_mode(native_controller_id),
            nullptr
        );
    }

    //  Re-read the controller.
    logger.log("Reading Controller Mode...");
    uint32_t type_id = read_controller_mode(botbase);
    status.current_controller = id_to_controller_type(type_id);
    logger.log(
        "Reading Controller Mode... Mode = " +
        CONTROLLER_TYPE_STRINGS.get_string(status.current_controller)
    );
}


void post_connect_read_switch1_wireless(
    ControllerModeStatus& status,
    const std::string& device_name,
    PABotBase& botbase,
    ControllerType desired_controller, uint32_t controller_id
){
    Logger& logger = botbase.logger();

    uint8_t controller_mac_address[6] = {};
    {
        BotBaseMessage response = botbase.issue_request_and_wait(
            DeviceRequest_read_mac_address(controller_id),
            nullptr
        );
        if (response.body.size() == sizeof(seqnum_t) + sizeof(controller_mac_address)){
            memcpy(
                controller_mac_address,
                response.body.data() + sizeof(seqnum_t),
                sizeof(controller_mac_address)
            );
        }else{
            logger.log(
                "Invalid response size to PABB_MSG_ESP32_REQUEST_READ_SPI: body = " + std::to_string(response.body.size()),
                COLOR_RED
            );
        }
    }

    NintendoSwitch::ControllerProfile profile =
        PokemonAutomation::NintendoSwitch::ConsoleSettings::instance().CONTROLLER_SETTINGS.get_or_make_profile(
            controller_mac_address,
            device_name,
            desired_controller
        );

    PABB_NintendoSwitch_ControllerColors colors;
    {
        Color color(profile.body_color);
        colors.body[0] = color.red();
        colors.body[1] = color.green();
        colors.body[2] = color.blue();
    }
    {
        Color color(profile.button_color);
        colors.buttons[0] = color.red();
        colors.buttons[1] = color.green();
        colors.buttons[2] = color.blue();
    }
    {
        Color color(profile.left_grip);
        colors.left_grip[0] = color.red();
        colors.left_grip[1] = color.green();
        colors.left_grip[2] = color.blue();
    }
    {
        Color color(profile.right_grip);
        colors.right_grip[0] = color.red();
        colors.right_grip[1] = color.green();
        colors.right_grip[2] = color.blue();
    }

    botbase.issue_request_and_wait(
        MessageControllerWriteSpi(
            desired_controller,
            0x00006050, sizeof(PABB_NintendoSwitch_ControllerColors),
            &colors
        ),
        nullptr
    );
}

void run_post_connect_actions_ESP32(
    ControllerModeStatus& status,
    const std::string& device_name,
    PABotBase& botbase,
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
    if (!change_controller){
        return;
    }

    ControllerType desired_controller = change_controller.value();
    switch (desired_controller){
//    case ControllerType::NintendoSwitch_WiredController:
    case ControllerType::NintendoSwitch_WirelessProController:
        post_connect_read_switch1_wireless(
            status, device_name, botbase, desired_controller,
            PABB_CID_NintendoSwitch_WirelessProController
        );
        break;

    case ControllerType::NintendoSwitch_LeftJoycon:
        post_connect_read_switch1_wireless(
            status, device_name, botbase, desired_controller,
            PABB_CID_NintendoSwitch_LeftJoycon
        );
        break;

    case ControllerType::NintendoSwitch_RightJoycon:
        post_connect_read_switch1_wireless(
            status, device_name, botbase, desired_controller,
            PABB_CID_NintendoSwitch_RightJoycon
        );
        break;

    default:;
    }

    uint32_t native_controller_id = controller_type_to_id(desired_controller);
    if (clear_settings){
        botbase.issue_request_and_wait(
            DeviceRequest_reset_to_controller(native_controller_id),
            nullptr
        );
    }else{
        botbase.issue_request_and_wait(
            DeviceRequest_change_controller_mode(native_controller_id),
            nullptr
        );
    }

    //  Re-read the controller.
    Logger& logger = botbase.logger();
    logger.log("Reading Controller Mode...");
    uint32_t type_id = read_controller_mode(botbase);
    status.current_controller = id_to_controller_type(type_id);
    logger.log(
        "Reading Controller Mode... Mode = " +
        CONTROLLER_TYPE_STRINGS.get_string(status.current_controller)
    );
}




void run_post_connect_actions(
    ControllerModeStatus& status,
    uint32_t program_id, const std::string& device_name,
    PABotBase& botbase,
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
    switch (program_id){
    case PABB_PID_PABOTBASE_ESP32:
        run_post_connect_actions_ESP32(status, device_name, botbase, change_controller, clear_settings);
        return;
    case PABB_PID_PABOTBASE_ESP32S3:
        run_post_connect_actions_ESP32S3(status, device_name, botbase, change_controller, clear_settings);
        return;
    }
}




}
}
