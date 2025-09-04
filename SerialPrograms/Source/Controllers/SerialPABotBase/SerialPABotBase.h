/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_H
#define PokemonAutomation_Controllers_SerialPABotBase_H

#include <stdint.h>
#include <string>
#include <map>
#include "Controllers/ControllerCapability.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



std::string program_name(uint8_t id);
ControllerType id_to_controller_type(uint32_t id);
uint32_t controller_type_to_id(ControllerType controller_type);



extern const std::map<
    uint32_t,   //  Protocol Version
    std::map<
        uint32_t,   //  Program ID
        std::map<ControllerType, ControllerFeatures>
    >
> SUPPORTED_VERSIONS;




}
}
#endif
