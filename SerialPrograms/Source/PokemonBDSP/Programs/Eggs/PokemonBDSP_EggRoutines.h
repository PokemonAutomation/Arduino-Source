/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggRoutines_H
#define PokemonAutomation_PokemonBDSP_EggRoutines_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void egg_spin(SwitchControllerContext& context, uint16_t duration);
void egg_spin_with_A(SwitchControllerContext& context, uint16_t duration);

void pickup_column(SwitchControllerContext& context);
void party_to_column(SwitchControllerContext& context, uint8_t column);
void column_to_party(SwitchControllerContext& context, uint8_t column);

void withdraw_1st_column_from_overworld(SwitchControllerContext& context);
void deposit_party_to_column(SwitchControllerContext& context, uint8_t column);
void swap_party(SwitchControllerContext& context, uint8_t current_column);


}
}
}
#endif
