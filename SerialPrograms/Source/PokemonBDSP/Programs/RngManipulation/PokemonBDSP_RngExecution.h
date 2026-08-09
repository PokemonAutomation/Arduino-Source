/*  BDSP RNG Execution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngExecution_H
#define PokemonAutomation_PokemonBDSP_RngExecution_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "Common/Cpp/Time.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonBDSP_RngTimeline.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{


struct PressSchedule{
    uint64_t span = 0;
    double starly_seconds = 0;
    double select_seconds = 0;
    double confirm_seconds = 0;
    uint64_t blinks_before_confirm = 0;
};

struct NavigationTimings{
    double ready_seconds = 0;
    double move_seconds = 0;
    double prompt_seconds = 0;
    double guard_seconds = 0.30;
};


void schedule_span_bounds(
    const BdspTimelineContext& timeline, size_t max_schedules,
    uint64_t& lowest, uint64_t& highest
);

std::vector<PressSchedule> schedule_presses(
    const Pokemon::Xorshift128State& state,
    const BdspTimelineContext& timeline,
    const NavigationTimings& timings,
    uint64_t press,
    size_t max_schedules
);


bool wait_until_moment(ProControllerContext& context, Logger& logger, WallClock when);


}
}
}
#endif
