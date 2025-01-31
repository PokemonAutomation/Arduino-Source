/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Soft reset, menus, etc.
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Navigation_H
#define PokemonAutomation_PokemonRSE_Navigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonRSE{

// Press A+B+Select+Start at the same time to soft reset, then re-enters the game.
// For now this assumes no dry battery.
void soft_reset(const ProgramInfo& info, VideoStream& stream, SwitchControllerContext &context);

// Run from battle. Cursor must start on the FIGHT button. Assumes fleeing will always work. (Smoke Ball)
void flee_battle(VideoStream& stream, SwitchControllerContext& context);

// After press A/walking up to enter a battle, run this handle the battle start and to check if opponent is shiny.
// Use flee_battle or soft_reset after this, depending on game.
bool handle_encounter(VideoStream& stream, SwitchControllerContext& context);


}
}
}
#endif
