/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_H
#define PokemonAutomation_Controllers_SerialPABotBase_H

#include <stdint.h>
#include <string>
#include <set>
#include <map>
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




const std::map<pabb_ProgramID, uint32_t>& SUPPORTED_DEVICES();
const std::map<uint32_t, std::set<pabb_ProgramID>>& SUPPORTED_VERSIONS();




std::string program_name(uint32_t id);
ControllerType id_to_controller_type(uint32_t id);
uint32_t controller_type_to_id(ControllerType controller_type);



}
}
#endif
