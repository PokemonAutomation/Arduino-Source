/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



const char NintendoSwitch_Basic[] = "NintendoSwitch-SerialPABotBase";


//  Defaults
const std::pair<std::string, std::set<ControllerFeature>> OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS{
    NintendoSwitch_Basic,
    {
        ControllerFeature::TickPrecise,
        ControllerFeature::NintendoSwitch_ProController,
    }
};








std::string program_name(uint8_t id){
    switch (id){
    case PABB_PID_UNSPECIFIED:      return "Microcontroller Program";
    case PABB_PID_PABOTBASE_12KB:   return "PABotBase-AVR8-12KB";
    case PABB_PID_PABOTBASE_31KB:   return "PABotBase-AVR8-31KB";
    default: return "Unknown ID";
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
    }},
};





}
}
