/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




//  Defaults
const ControllerRequirements OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS{
    ControllerFeature::TickPrecise,
    ControllerFeature::NintendoSwitch_ProController,
};




std::string program_name(uint8_t id){
    switch (id){
    case PABB_PID_UNSPECIFIED:      return "Microcontroller Program";
    case PABB_PID_PABOTBASE_12KB:   return "PABotBase-AVR8-12KB";
    case PABB_PID_PABOTBASE_31KB:   return "PABotBase-AVR8-31KB";
    case PABB_PID_PABOTBASE_ESP32:  return "PABotBase-ESP32";
    default: return "Unknown ID";
    }
}
ControllerType controller_type(uint32_t id){
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
    default:
        return ControllerType::None;
    }
}



const std::map<
    uint32_t,   //  Protocol Version
    std::map<
        uint32_t,   //  Program ID
        std::map<ControllerType, std::set<ControllerFeature>>
    >
> SUPPORTED_VERSIONS{
    {2021052600, {
        {PABB_PID_UNSPECIFIED, {{ControllerType::None, {}}}},
        {PABB_PID_PABOTBASE_12KB, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
            }},
        }},
        {PABB_PID_PABOTBASE_31KB, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
    }},
    {2023121900, {
        {PABB_PID_UNSPECIFIED, {{ControllerType::None, {}}}},
        {PABB_PID_PABOTBASE_12KB, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::QueryCommandQueueSize,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_31KB, {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::QueryCommandQueueSize,
                ControllerFeature::NintendoSwitch_ProController,
                ControllerFeature::NintendoSwitch_DateSkip,
            }},
        }},
        {PABB_PID_PABOTBASE_ESP32, {
            {ControllerType::NintendoSwitch_WirelessProController, {
                ControllerFeature::TickPrecise,
                ControllerFeature::QueryCommandQueueSize,
                ControllerFeature::NintendoSwitch_ProController,
            }},
            {ControllerType::NintendoSwitch_LeftJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::QueryCommandQueueSize,
            }},
            {ControllerType::NintendoSwitch_RightJoycon, {
                ControllerFeature::TickPrecise,
                ControllerFeature::QueryCommandQueueSize,
            }},
        }},
    }},
};










}
}
