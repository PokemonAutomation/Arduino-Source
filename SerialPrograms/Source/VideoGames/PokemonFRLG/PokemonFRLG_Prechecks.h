/*  FRLG Prechecks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The prechecks FRLG programs share. See NintendoSwitch_Prechecks.h.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_Prechecks_H
#define PokemonAutomation_PokemonFRLG_Prechecks_H

#include <cstddef>
#include <memory>
#include "NintendoSwitch/Programs/NintendoSwitch_Prechecks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


//  "black_border": goes Home to check the screen size and the controller,
//  then resumes the game. ALWAYS_RUNS.
std::unique_ptr<const Precheck> make_black_border_precheck();

//  "free_party_slots": opens the party menu from the overworld and checks
//  that at least free_slots_needed slots are free. SKIPPABLE.
std::unique_ptr<const Precheck> make_free_party_slots_precheck(size_t free_slots_needed);

//  The free_party_slots verdict for a party of party_size Pokemon.
PrecheckOutcome free_party_slots_outcome(size_t party_size, size_t free_slots_needed);


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
