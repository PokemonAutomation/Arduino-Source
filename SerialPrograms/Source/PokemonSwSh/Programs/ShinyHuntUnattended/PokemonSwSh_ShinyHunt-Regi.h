/*  Regi Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiPatterns_H
#define PokemonAutomation_PokemonSwSh_RegiPatterns_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Time.h"
#include "PokemonSwSh/Options/PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ProControllerContext;
namespace PokemonSwSh{


void move_to_corner(
    Logger& logger, ProControllerContext& context,
    bool correction, Milliseconds TRANSITION_DELAY
);

void run_regi_light_puzzle(
    Logger& logger, ProControllerContext& context,
    RegiGolem regi, uint64_t encounter
);



}
}
}
#endif
