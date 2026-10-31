// Shared battle helpers used by AutoStory routes.
#pragma once

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {
// Reuse the Oreburgh Gate down/right entry search. Returns at fade start;
// caller owns destination-specific fade completion and interior confirmation.
// north_fallback preserves Oreburgh's final north sweep when explicitly set.
void enter_cave_with_down_right_search(VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    Milliseconds north_fallback = Milliseconds(0));

// Shared confirmed-Run escape: verifies exit transition or main menu.
void escape_wild_battle(VideoStream& stream,
    ProControllerContext& context,
    BattleType battle_type = BattleType::STANDARD,
    std::chrono::seconds timeout = std::chrono::seconds(30));

// Shared legacy first-move battle routine; consolidation remains preliminary.
void Barry_1_fight(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context);
}
}
}
