/*  Regi Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiPatterns_H
#define PokemonAutomation_PokemonSwSh_RegiPatterns_H

#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_to_corner(
    SingleSwitchProgramEnvironment& env,
    bool correction, uint16_t TRANSITION_DELAY
);

void run_regi_light_puzzle(
    SingleSwitchProgramEnvironment& env,
    RegiGolem regi, uint64_t encounter
);



}
}
}
#endif
