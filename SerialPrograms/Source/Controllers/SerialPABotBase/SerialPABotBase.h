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
#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




const std::map<pabb_ProgramID, uint32_t>& SUPPORTED_DEVICES();
const std::map<
    uint32_t,   //  Major protocol version. (version # / 100)
    std::map<
        pabb_ProgramID,
        uint8_t //  Minimum minor protocol version (version # % 100)
    >
>& SUPPORTED_VERSIONS();




ControllerType id_to_controller_type(uint32_t id);
uint32_t controller_type_to_id(ControllerType controller_type);



}
}
#endif
