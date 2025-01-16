/*  Heal the party using Global Room
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GlobalRoomHeal_H
#define PokemonAutomation_PokemonBDSP_GlobalRoomHeal_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonBDSP{


// Use Global Room to heal the party.
// Must start at overworld and have Y-shotcut to Global Room unlocked.
bool heal_by_global_room(ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif
