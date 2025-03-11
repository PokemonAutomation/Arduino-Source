/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggRoutines_H
#define PokemonAutomation_PokemonBDSP_EggRoutines_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void egg_spin(ProControllerContext& context, Milliseconds duration);
void egg_spin_with_A(ProControllerContext& context, Milliseconds duration);

void pickup_column(ProControllerContext& context);
void party_to_column(ProControllerContext& context, uint8_t column);
void column_to_party(ProControllerContext& context, uint8_t column);

void withdraw_1st_column_from_overworld(ProControllerContext& context);
void deposit_party_to_column(ProControllerContext& context, uint8_t column);
void swap_party(ProControllerContext& context, uint8_t current_column);


}
}
}
#endif
