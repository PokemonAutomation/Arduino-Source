/*  Rng Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngNavigation_H
#define PokemonAutomation_PokemonFRLG_RngNavigation_H

#include "PokemonFRLG_BlindNavigation.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonFRLG{

    
int watch_for_shiny_encounter(ConsoleHandle& console, ProControllerContext& context);

bool check_for_shiny(ConsoleHandle& console, ProControllerContext& context, PokemonFRLG_RngTarget TARGET);


}
}
}
#endif
