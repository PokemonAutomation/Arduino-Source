/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Soft reset, menus, etc.
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Navigation_H
#define PokemonAutomation_PokemonRSE_Navigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonRSE{

// Press A+B+Select+Start at the same time to soft reset, then re-enter the game.
// This is intended for R/S (E's RNG is broken) and assumes that there is no dry battery.
// For R/S, the initial seed is based on the real time clock and changes every minute
// A randomized wait is used after loading in to avoid hitting the same frame
uint64_t soft_reset(ConsoleHandle& console, ProControllerContext& context);

// Run from battle. Cursor must start on the FIGHT button. Assumes fleeing will always work. (Smoke Ball)
void flee_battle(VideoStream& stream, ProControllerContext& context);

// After press A/walking up to enter a battle, run this handle the battle start and to check if opponent is shiny.
// Set send_out_lead to true and then use flee_battle() after if game is Emerald.
// For R/S, send_out_lead as false and then soft_reset() to save time.
bool handle_encounter(ConsoleHandle& console, ProControllerContext& context, bool send_out_lead);

// Go to home to check that scaling is 100%. Then resume game.
// Skips checks if not Switch.
void home_black_border_check(ConsoleHandle& console, ProControllerContext& context);


}
}
}
#endif
