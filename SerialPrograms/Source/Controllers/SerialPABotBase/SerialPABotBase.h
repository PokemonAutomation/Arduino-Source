/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_H
#define PokemonAutomation_Controllers_SerialPABotBase_H

#include <stdint.h>
#include <string>
#include <set>
#include <map>
#include "Controllers/ControllerCapability.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


//  Supported Controllers
extern const char NintendoSwitch_Basic[];



//  Defaults
extern const ControllerRequirements OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS;



std::string program_name(uint8_t id);




extern const std::map<
    uint32_t,   //  Protocol Version
    std::map<
        uint32_t,   //  Program ID
        std::map<ControllerType, std::set<ControllerFeature>>
    >
> SUPPORTED_VERSIONS;



}
}
#endif
