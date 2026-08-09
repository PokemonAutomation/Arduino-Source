/*  BDSP RNG Execution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <chrono>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_RngExecution.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



static size_t first_free_blink(const BdspTimelineContext& timeline){
    size_t before_blinks = 1 + (timeline.plus_one_on_menu_close ? 1 : 0)
        + timeline.advance_delay + timeline.pokemon_models;
    size_t last_waited = before_blinks + timeline.advance_delay_2_after_events - 1;
    return last_waited + 1 + timeline.advance_delay_2;
}


static size_t timeline_depth(const BdspTimelineContext& timeline, size_t max_schedules){
    return first_free_blink(timeline) + 4 * max_schedules + 2;
}

void schedule_span_bounds(
    const BdspTimelineContext& timeline, size_t max_schedules,
    uint64_t& lowest, uint64_t& highest
){
    lowest = first_free_blink(timeline) + 1 + timeline.advances_after_accept;
    highest = timeline_depth(timeline, max_schedules) - 1 + timeline.advances_after_accept;
}


std::vector<PressSchedule> schedule_presses(
    const Pokemon::Xorshift128State& state,
    const BdspTimelineContext& timeline,
    const NavigationTimings& timings,
    uint64_t press,
    size_t max_schedules
){
    std::vector<PressSchedule> schedules;

    size_t first_free = first_free_blink(timeline);
    size_t last_waited = first_free - 1 - timeline.advance_delay_2;

    Pokemon::Xorshift128 rng(state);
    rng.advance(press);
    BdspTimelineResult simulated = simulate_timeline(
        rng, timeline, timeline_depth(timeline, max_schedules)
    );
    const std::vector<double>& at = simulated.advance_times;
    if (at.size() < first_free + 2){
        return schedules;
    }

    //  The second press goes between the last blink it waits for and the next.
    double starly = (at[last_waited] + at[last_waited + 1]) / 2;
    if (starly < timings.ready_seconds){
        return schedules;
    }
    double earliest_select = starly + timings.move_seconds;

    for (size_t blink = first_free;
        blink + 1 < at.size() && schedules.size() < max_schedules;
        blink++
    ){
        double gap_start = at[blink];
        double gap_end = at[blink + 1];

        //  Centred in the gap when there is room, pushed as late as the navigation
        //  demands when there is not.
        double select_at = std::max(
            (gap_start + gap_end - timings.prompt_seconds) / 2, earliest_select
        );
        double confirm_at = select_at + timings.prompt_seconds;
        if (select_at - gap_start < timings.guard_seconds
            || gap_end - confirm_at < timings.guard_seconds
        ){
            continue;
        }

        schedules.push_back(PressSchedule{
            /*span*/                  blink + 1 + timeline.advances_after_accept,
            /*starly_seconds*/        starly,
            /*select_seconds*/        select_at,
            /*confirm_seconds*/       confirm_at,
            /*blinks_before_confirm*/ blink + 1 - first_free,
        });
    }

    return schedules;
}


bool wait_until_moment(ProControllerContext& context, Logger& logger, WallClock when){
    context.wait_for_all_requests();

    WallClock now = current_time();
    if (when < now){
        double late = std::chrono::duration_cast<std::chrono::duration<double>>(
            now - when
        ).count();
        logger.log(
            "The moment passed " + tostr_fixed(late, 3) + " seconds ago. "
            "Abandoning the attempt rather than starting late.",
            COLOR_ORANGE
        );
        return false;
    }

    Milliseconds wait = std::chrono::duration_cast<Milliseconds>(when - now);
    pbf_wait(context, wait);
    logger.log("Waiting " + std::to_string(wait.count()) + " ms for the timeline.",
        COLOR_BLUE);
    return true;
}




}
}
}
