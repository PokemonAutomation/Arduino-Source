// Shared battle helpers used by AutoStory routes.
#pragma once

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"

namespace PokemonAutomation { namespace NintendoSwitch { namespace PokemonBDSP {
// Shared confirmed-Run escape: verifies exit transition or main menu.
void escape_wild_battle(VideoStream& stream, ProControllerContext& context,
    BattleType battle_type = BattleType::STANDARD,
    std::chrono::seconds timeout = std::chrono::seconds(30));

// Shared legacy first-move battle routine; consolidation remains preliminary.
void Barry_1_fight(SingleSwitchProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context);
}}}
