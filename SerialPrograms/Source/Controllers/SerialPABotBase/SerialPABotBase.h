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
#include "Common/PokemonSwSh/PokemonProgramIDs.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


extern const char INTERFACE_NAME[];


enum class Features{
    TickPrecise,
    NintendoSwitch_Basic,
    NintendoSwitch_Macros,
    NintendoSwitch_DateSkip,
};


const char* to_string(Features feature);


std::set<std::string> program_id_to_features(uint8_t id);

extern const std::pair<std::string, std::set<std::string>> OLD_SERIAL_DEFAULT;





};





}
#endif
