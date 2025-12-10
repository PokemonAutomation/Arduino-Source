/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



const std::map<pabb_ProgramID, uint32_t>& SUPPORTED_DEVICES(){
    static const std::map<pabb_ProgramID, uint32_t> database{
        {PABB_PID_UNSPECIFIED,                  2025090400},
        {PABB_PID_PABOTBASE_ArduinoUnoR3,       2025090303},
        {PABB_PID_PABOTBASE_ArduinoLeonardo,    2025090303},
        {PABB_PID_PABOTBASE_ProMicro,           2025090303},
        {PABB_PID_PABOTBASE_Teensy2,            2025090303},
        {PABB_PID_PABOTBASE_TeensyPP2,          2025090303},
        {PABB_PID_PABOTBASE_ESP32,              2025120800},
        {PABB_PID_PABOTBASE_ESP32S3,            2025120800},
        {PABB_PID_PABOTBASE_Pico1W_USB,         2025120800},
        {PABB_PID_PABOTBASE_Pico1W_UART,        2025120800},
        {PABB_PID_PABOTBASE_Pico2W_USB,         2025120800},
        {PABB_PID_PABOTBASE_Pico2W_UART,        2025120800},
    };
    return database;
}

std::map<uint32_t, std::map<pabb_ProgramID, uint8_t>> make_SUPPORTED_VERSIONS(){
    std::map<uint32_t, std::map<pabb_ProgramID, uint8_t>> ret;
    for (const auto& item : SUPPORTED_DEVICES()){
        ret[item.second / 100][item.first] = (uint8_t)(item.second % 100);
    }
    return ret;
}
const std::map<uint32_t, std::map<pabb_ProgramID, uint8_t>>& SUPPORTED_VERSIONS(){
    static const std::map<uint32_t, std::map<pabb_ProgramID, uint8_t>> database = make_SUPPORTED_VERSIONS();
    return database;
}


bool controller_is_valid(uint32_t id){
    switch (id){
    case PABB_CID_NONE:
    case PABB_CID_StandardHid_Keyboard:
    case PABB_CID_NintendoSwitch_WiredController:
    case PABB_CID_NintendoSwitch2_WiredController:
    case PABB_CID_NintendoSwitch_WirelessProController:
    case PABB_CID_NintendoSwitch_WirelessLeftJoycon:
    case PABB_CID_NintendoSwitch_WirelessRightJoycon:
    case PABB_CID_NintendoSwitch_WiredProController:
    case PABB_CID_NintendoSwitch_WiredLeftJoycon:
    case PABB_CID_NintendoSwitch_WiredRightJoycon:
        return true;
//        return IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE;
    case PABB_CID_NintendoSwitch2_WiredProController:
    case PABB_CID_NintendoSwitch2_WiredLeftJoycon:
    case PABB_CID_NintendoSwitch2_WiredRightJoycon:
    case PABB_CID_NintendoSwitch2_WirelessProController:
    case PABB_CID_NintendoSwitch2_WirelessLeftJoycon:
    case PABB_CID_NintendoSwitch2_WirelessRightJoycon:
        return PreloadSettings::instance().DEVELOPER_MODE;
    }
    return false;
}



ControllerType id_to_controller_type(uint32_t id){
    switch (id){
    case PABB_CID_NONE:
        return ControllerType::None;

    case PABB_CID_StandardHid_Keyboard:
        return ControllerType::HID_Keyboard;

    case PABB_CID_NintendoSwitch_WiredController:
        return ControllerType::NintendoSwitch_WiredController;
    case PABB_CID_NintendoSwitch_WiredProController:
        return ControllerType::NintendoSwitch_WiredProController;
    case PABB_CID_NintendoSwitch_WiredLeftJoycon:
        return ControllerType::NintendoSwitch_WiredLeftJoycon;
    case PABB_CID_NintendoSwitch_WiredRightJoycon:
        return ControllerType::NintendoSwitch_WiredRightJoycon;
    case PABB_CID_NintendoSwitch_WirelessProController:
        return ControllerType::NintendoSwitch_WirelessProController;
    case PABB_CID_NintendoSwitch_WirelessLeftJoycon:
        return ControllerType::NintendoSwitch_WirelessLeftJoycon;
    case PABB_CID_NintendoSwitch_WirelessRightJoycon:
        return ControllerType::NintendoSwitch_WirelessRightJoycon;

    case PABB_CID_NintendoSwitch2_WiredController:
        return ControllerType::NintendoSwitch2_WiredController;
    case PABB_CID_NintendoSwitch2_WiredProController:
        return ControllerType::NintendoSwitch2_WiredProController;
    case PABB_CID_NintendoSwitch2_WiredLeftJoycon:
        return ControllerType::NintendoSwitch2_WiredLeftJoycon;
    case PABB_CID_NintendoSwitch2_WiredRightJoycon:
        return ControllerType::NintendoSwitch2_WiredRightJoycon;
    case PABB_CID_NintendoSwitch2_WirelessProController:
        return ControllerType::NintendoSwitch2_WirelessProController;
    case PABB_CID_NintendoSwitch2_WirelessLeftJoycon:
        return ControllerType::NintendoSwitch2_WirelessLeftJoycon;
    case PABB_CID_NintendoSwitch2_WirelessRightJoycon:
        return ControllerType::NintendoSwitch2_WirelessRightJoycon;
    }

    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid Controller ID: " + std::to_string(id)
    );
}
uint32_t controller_type_to_id(ControllerType controller_type){
    switch (controller_type){
    case ControllerType::None:
        return PABB_CID_NONE;

    case ControllerType::HID_Keyboard:
        return PABB_CID_StandardHid_Keyboard;

    case ControllerType::NintendoSwitch_WiredController:
        return PABB_CID_NintendoSwitch_WiredController;
    case ControllerType::NintendoSwitch_WiredProController:
        return PABB_CID_NintendoSwitch_WiredProController;
    case ControllerType::NintendoSwitch_WiredLeftJoycon:
        return PABB_CID_NintendoSwitch_WiredLeftJoycon;
    case ControllerType::NintendoSwitch_WiredRightJoycon:
        return PABB_CID_NintendoSwitch_WiredRightJoycon;
    case ControllerType::NintendoSwitch_WirelessProController:
        return PABB_CID_NintendoSwitch_WirelessProController;
    case ControllerType::NintendoSwitch_WirelessLeftJoycon:
        return PABB_CID_NintendoSwitch_WirelessLeftJoycon;
    case ControllerType::NintendoSwitch_WirelessRightJoycon:
        return PABB_CID_NintendoSwitch_WirelessRightJoycon;

    case ControllerType::NintendoSwitch2_WiredController:
        return PABB_CID_NintendoSwitch2_WiredController;
    case ControllerType::NintendoSwitch2_WiredProController:
        return PABB_CID_NintendoSwitch2_WiredProController;
    case ControllerType::NintendoSwitch2_WiredLeftJoycon:
        return PABB_CID_NintendoSwitch2_WiredLeftJoycon;
    case ControllerType::NintendoSwitch2_WiredRightJoycon:
        return PABB_CID_NintendoSwitch2_WiredRightJoycon;
    case ControllerType::NintendoSwitch2_WirelessProController:
        return PABB_CID_NintendoSwitch2_WirelessProController;
    case ControllerType::NintendoSwitch2_WirelessLeftJoycon:
        return PABB_CID_NintendoSwitch2_WirelessLeftJoycon;
    case ControllerType::NintendoSwitch2_WirelessRightJoycon:
        return PABB_CID_NintendoSwitch2_WirelessRightJoycon;
    }

    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid Controller Enum: " + std::to_string((int)controller_type)
    );
}








}
}
