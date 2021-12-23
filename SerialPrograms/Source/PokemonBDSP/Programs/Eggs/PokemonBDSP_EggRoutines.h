/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggRoutines_H
#define PokemonAutomation_PokemonBDSP_EggRoutines_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void egg_spin(const BotBaseContext& context, uint16_t duration);
void egg_spin_with_A(const BotBaseContext& context, uint16_t duration);

void pickup_column(const BotBaseContext& context);
void party_to_column(const BotBaseContext& context, uint8_t column);
void column_to_party(const BotBaseContext& context, uint8_t column);

void withdraw_1st_column_from_overworld(const BotBaseContext& context);
void deposit_party_to_column(const BotBaseContext& context, uint8_t column);
void swap_party(const BotBaseContext& context, uint8_t current_column);


}
}
}
#endif
