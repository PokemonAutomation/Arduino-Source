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




//  Defaults
const ControllerFeatures OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS{
    ControllerFeature::TickPrecise,
    ControllerFeature::NintendoSwitch_ProController,
};




std::string program_name(uint8_t id){
    switch (id){
    case PABB_PID_UNSPECIFIED:                  return "Microcontroller Program";

    //  Old (fat) PABotBase with scheduler.
    case PABB_PID_PABOTBASE_12KB:               return "PABotBase-AVR8-12KB";
    case PABB_PID_PABOTBASE_31KB:               return "PABotBase-AVR8-31KB";

    //  New (slim) PABotBase.
    case PABB_PID_PABOTBASE_ArduinoUnoR3:       return "PABotBase-UnoR3";
    case PABB_PID_PABOTBASE_ArduinoLeonardo:    return "PABotBase-Leonardo";
    case PABB_PID_PABOTBASE_ProMicro:           return "PABotBase-ProMicro";
    case PABB_PID_PABOTBASE_Teensy2:            return "PABotBase-Teensy2.0";
    case PABB_PID_PABOTBASE_TeensyPP2:          return "PABotBase-Teensy++2.0";

    case PABB_PID_PABOTBASE_CH552:              return "PABotBase-CH552";

    case PABB_PID_PABOTBASE_ESP32:              return "PABotBase-ESP32";
    case PABB_PID_PABOTBASE_ESP32S3:            return "PABotBase-ESP32-S3";

    default: return "Unknown ID";
    }
}
ControllerType id_to_controller_type(uint32_t id){
    switch (id){
    case PABB_CID_NONE:
        return ControllerType::None;
    case PABB_CID_NINTENDO_SWITCH_WIRED_PRO_CONTROLLER:
        return ControllerType::NintendoSwitch_WiredProController;
    case PABB_CID_NINTENDO_SWITCH_WIRELESS_PRO_CONTROLLER:
        return ControllerType::NintendoSwitch_WirelessProController;
    case PABB_CID_NINTENDO_SWITCH_LEFT_JOYCON:
        return ControllerType::NintendoSwitch_LeftJoycon;
    case PABB_CID_NINTENDO_SWITCH_RIGHT_JOYCON:
        return ControllerType::NintendoSwitch_RightJoycon;
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
    case ControllerType::NintendoSwitch_WiredProController:
        return PABB_CID_NINTENDO_SWITCH_WIRED_PRO_CONTROLLER;
    case ControllerType::NintendoSwitch_WirelessProController:
        return PABB_CID_NINTENDO_SWITCH_WIRELESS_PRO_CONTROLLER;
    case ControllerType::NintendoSwitch_LeftJoycon:
        return PABB_CID_NINTENDO_SWITCH_LEFT_JOYCON;
    case ControllerType::NintendoSwitch_RightJoycon:
        return PABB_CID_NINTENDO_SWITCH_RIGHT_JOYCON;
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid Controller Enum: " + std::to_string((int)controller_type)
    );
}



const std::map<
    uint32_t,   //  Protocol Version
    std::map<
        uint32_t,   //  Program ID
        std::map<ControllerType, ControllerFeatures>
    >
> SUPPORTED_VERSIONS{
    {2025061202, {
        {PABB_PID_PABOTBASE_ESP32, {
            {ControllerType::NintendoSwitch_WirelessProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_LeftJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_LeftJoycon,
            }},
            {ControllerType::NintendoSwitch_RightJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_RightJoycon,
            }},
        }},
    }},
    {2025061304, {
        {PABB_PID_PABOTBASE_ESP32S3, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
    }},
    {2025061404, {
        {PABB_PID_PABOTBASE_ArduinoUnoR3, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_ArduinoLeonardo, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_ProMicro, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_Teensy2, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_TeensyPP2, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::TimingFlexibleMilliseconds,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
    }},
};










}
}
