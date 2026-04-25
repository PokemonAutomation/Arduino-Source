/*  Hard Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_HardReset_H
#define PokemonAutomation_PokemonFRLG_HardReset_H


#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonFRLG_BlindNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

void reset_and_perform_blind_sequence(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    SeedButton SEED_BUTTON,
    uint64_t SEED_DELAY, 
    uint64_t CONTINUE_SCREEN_DELAY, 
    uint64_t TEACHY_DELAY, 
    uint64_t INGAME_DELAY, 
    bool SAFARI_ZONE,
    uint8_t PROFILE
);

#if 0
void reset_and_detect_copyright_text(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    uint8_t PROFILE = 0
);
#endif

}
}
}
#endif



