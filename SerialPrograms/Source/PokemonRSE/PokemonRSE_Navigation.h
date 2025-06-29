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
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonRSE{

// Press A+B+Select+Start at the same time to soft reset, then re-enters the game.
// For now this assumes no dry battery.
void soft_reset(const ProgramInfo& info, VideoStream& stream, ProControllerContext &context);

// Run from battle. Cursor must start on the FIGHT button. Assumes fleeing will always work. (Smoke Ball)
void flee_battle(VideoStream& stream, ProControllerContext& context);

// After press A/walking up to enter a battle, run this handle the battle start and to check if opponent is shiny.
// Set send_out_lead to true and then use flee_battle() after if game is Emerald.
// For R/S, send_out_lead as false and then soft_reset() to save time.
bool handle_encounter(VideoStream& stream, ProControllerContext& context, bool send_out_lead);


}
}
}
#endif
