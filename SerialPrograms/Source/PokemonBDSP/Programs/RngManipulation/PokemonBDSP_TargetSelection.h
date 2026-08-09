/*  BDSP Target Selection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_TargetSelection_H
#define PokemonAutomation_PokemonBDSP_TargetSelection_H

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <string>
#include "Pokemon/Pokemon_BdspRng.h"
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP_RngExecution.h"
#include "PokemonBDSP_RngTimeline.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct TargetSearchRequest{
    Pokemon::Xorshift128State state;
    BdspTimelineContext timeline;
    NavigationTimings timings;
    Pokemon::BdspStaticTemplate pokemon;
    uint8_t npcs = 1;
    double tick_seconds = BDSP_NPC_TICK_SECONDS;
    int64_t bias = 0;
    uint64_t first_press = 0;
    uint64_t window_advances = 0;
    size_t max_schedules = 8;
    std::function<bool(const Pokemon::BdspPokemonResult& result)> wanted;
};


struct TargetSelectionResult{
    bool success = false;
    uint64_t press_advance = 0;
    uint64_t target_advance = 0;
    PressSchedule schedule;
    Pokemon::BdspPokemonResult target;
    size_t matches_found = 0;
    uint64_t advances_scanned = 0;
    std::string failure_reason;
};


TargetSelectionResult select_target(const TargetSearchRequest& request);


}
}
}
#endif
