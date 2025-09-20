/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



const std::map<pabb_ProgramID, uint32_t>& SUPPORTED_DEVICES(){
    static const std::map<pabb_ProgramID, uint32_t> database{
        {PABB_PID_UNSPECIFIED,                  2025090400},
        {PABB_PID_PABOTBASE_ArduinoUnoR3,       2025090300},
        {PABB_PID_PABOTBASE_ArduinoLeonardo,    2025090300},
        {PABB_PID_PABOTBASE_ProMicro,           2025090300},
        {PABB_PID_PABOTBASE_Teensy2,            2025090300},
        {PABB_PID_PABOTBASE_TeensyPP2,          2025090300},
        {PABB_PID_PABOTBASE_ESP32,              2025090302},
        {PABB_PID_PABOTBASE_ESP32S3,            2025090300},
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
    case PABB_CID_NintendoSwitch_WirelessProController:
        return ControllerType::NintendoSwitch_WirelessProController;
    case PABB_CID_NintendoSwitch_LeftJoycon:
        return ControllerType::NintendoSwitch_LeftJoycon;
    case PABB_CID_NintendoSwitch_RightJoycon:
        return ControllerType::NintendoSwitch_RightJoycon;

    case PABB_CID_NintendoSwitch2_WiredController:
        return ControllerType::NintendoSwitch2_WiredController;
    case PABB_CID_NintendoSwitch2_WiredProController:
        return ControllerType::NintendoSwitch2_WiredProController;
    case PABB_CID_NintendoSwitch2_WirelessProController:
        return ControllerType::NintendoSwitch2_WirelessProController;
    case PABB_CID_NintendoSwitch2_LeftJoycon:
        return ControllerType::NintendoSwitch2_LeftJoycon;
    case PABB_CID_NintendoSwitch2_RightJoycon:
        return ControllerType::NintendoSwitch2_RightJoycon;

    default:
        return ControllerType::None;
    }

//    throw InternalProgramError(
//        nullptr, PA_CURRENT_FUNCTION,
//        "Invalid Controller ID: " + std::to_string(id)
//    );
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
    case ControllerType::NintendoSwitch_WirelessProController:
        return PABB_CID_NintendoSwitch_WirelessProController;
    case ControllerType::NintendoSwitch_LeftJoycon:
        return PABB_CID_NintendoSwitch_LeftJoycon;
    case ControllerType::NintendoSwitch_RightJoycon:
        return PABB_CID_NintendoSwitch_RightJoycon;

    case ControllerType::NintendoSwitch2_WiredController:
        return PABB_CID_NintendoSwitch2_WiredController;
    case ControllerType::NintendoSwitch2_WiredProController:
        return PABB_CID_NintendoSwitch2_WiredProController;
    case ControllerType::NintendoSwitch2_WirelessProController:
        return PABB_CID_NintendoSwitch2_WirelessProController;
    case ControllerType::NintendoSwitch2_LeftJoycon:
        return PABB_CID_NintendoSwitch2_LeftJoycon;
    case ControllerType::NintendoSwitch2_RightJoycon:
        return PABB_CID_NintendoSwitch2_RightJoycon;
    }

    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid Controller Enum: " + std::to_string((int)controller_type)
    );
}








}
}
