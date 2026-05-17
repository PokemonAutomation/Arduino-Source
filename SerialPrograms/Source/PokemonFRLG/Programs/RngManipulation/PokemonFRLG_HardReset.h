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
    PokemonFRLG_RngTarget target,
    const SeedButton& seed_button,
    const BlackoutButton& extra_button,
    const RngTimings& timings,
    bool safari_zone,
    uint8_t profile
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



