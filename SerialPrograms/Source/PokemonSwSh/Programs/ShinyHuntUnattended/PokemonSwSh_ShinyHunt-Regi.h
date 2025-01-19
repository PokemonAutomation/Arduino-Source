/*  Regi Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiPatterns_H
#define PokemonAutomation_PokemonSwSh_RegiPatterns_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_to_corner(
    Logger& logger, SwitchControllerContext& context,
    bool correction, uint16_t TRANSITION_DELAY
);

void run_regi_light_puzzle(
    Logger& logger, SwitchControllerContext& context,
    RegiGolem regi, uint64_t encounter
);



}
}
}
#endif
