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
    case PABB_PID_UNSPECIFIED:                  return "None";

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

    case PABB_PID_PABOTBASE_PicoW_USB:          return "PABotBase-PicoW (USB)";
    case PABB_PID_PABOTBASE_PicoW_UART:         return "PABotBase-PicoW (UART)";

    default: return "Unknown ID";
    }
}
ControllerType id_to_controller_type(uint32_t id){
    switch (id){
    case PABB_CID_NONE:
        return ControllerType::None;

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



const std::map<
    uint32_t,   //  Protocol Version
    std::map<
        uint32_t,   //  Program ID
        std::map<ControllerType, ControllerFeatures>
    >
> SUPPORTED_VERSIONS{
    {2025081300, {
        {PABB_PID_PABOTBASE_ArduinoUnoR3, {
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
        {PABB_PID_PABOTBASE_ArduinoLeonardo, {
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
        {PABB_PID_PABOTBASE_ProMicro, {
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
        {PABB_PID_PABOTBASE_Teensy2, {
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
        {PABB_PID_PABOTBASE_TeensyPP2, {
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
    }},
    {2025081700, {
        {PABB_PID_PABOTBASE_ESP32, {
            {ControllerType::NintendoSwitch_WirelessProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_LeftJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_LeftJoycon,
            }},
            {ControllerType::NintendoSwitch_RightJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_RightJoycon,
            }},
        }},
        {PABB_PID_PABOTBASE_PicoW_USB, {
            {ControllerType::NintendoSwitch_WirelessProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_LeftJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_LeftJoycon,
            }},
            {ControllerType::NintendoSwitch_RightJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_RightJoycon,
            }},
        }},
        {PABB_PID_PABOTBASE_PicoW_UART, {
            {ControllerType::NintendoSwitch_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_WirelessProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_LeftJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_LeftJoycon,
            }},
            {ControllerType::NintendoSwitch_RightJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_RightJoycon,
            }},
        }},
    }},
    {2025081711, {
        {PABB_PID_PABOTBASE_ESP32S3, {
            {ControllerType::NintendoSwitch_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch2_WiredController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
    }},
};










}
}
