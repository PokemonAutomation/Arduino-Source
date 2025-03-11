/*  Heal the party using Global Room
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GlobalRoomHeal_H
#define PokemonAutomation_PokemonBDSP_GlobalRoomHeal_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// Use Global Room to heal the party.
// Must start at overworld and have Y-shotcut to Global Room unlocked.
bool heal_by_global_room(VideoStream& stream, ProControllerContext& context);


}
}
}
#endif
