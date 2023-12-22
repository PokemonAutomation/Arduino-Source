/*  Misc. Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_Misc_H
#define PokemonAutomation_PokemonSwSh_Commands_Misc_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//void mash_A         (BotBaseContext& context, uint16_t ticks);
void IoA_backout    (BotBaseContext& context, uint16_t pokemon_to_menu_delay);



}
}
#endif
