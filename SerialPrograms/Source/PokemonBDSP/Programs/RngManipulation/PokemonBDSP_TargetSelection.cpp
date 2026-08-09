/*  BDSP Target Selection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include <set>
#include "PokemonBDSP_RngAim.h"
#include "PokemonBDSP_TargetSelection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


TargetSelectionResult select_target(const TargetSearchRequest& request){
    TargetSelectionResult result;

    if (!request.wanted){
        result.failure_reason = "No filter was supplied, so nothing can be searched for.";
        return result;
    }

    BdspStaticSearcher searcher(request.state, request.pokemon, 0);

    //  How many advances the scene can spend between the press and the generation
    uint64_t lowest_span = 0;
    uint64_t highest_span = 0;
    schedule_span_bounds(request.timeline, request.max_schedules, lowest_span, highest_span);

    int64_t last_press = (int64_t)(request.first_press + request.window_advances);

    //  One sweep, walking the RNG forward rather than jumping to each advance.
    uint64_t scan_from = (uint64_t)std::max<int64_t>(
        0, (int64_t)request.first_press + (int64_t)lowest_span + request.bias
    );
    uint64_t scan_to = (uint64_t)std::max<int64_t>(
        0, last_press + (int64_t)highest_span + request.bias
    );
    std::map<uint64_t, BdspPokemonResult> wanted_targets;
    for (const BdspRngHit& hit : searcher.scan(scan_from, scan_to, request.wanted)){
        wanted_targets.emplace(hit.advances, hit.result);
    }

    result.advances_scanned = scan_to - scan_from + 1;
    result.matches_found = wanted_targets.size();

    if (wanted_targets.empty()){
        result.failure_reason = "no matches found within "
            + std::to_string(result.advances_scanned) + " advances.";
        return result;
    }

    uint8_t press_step = request.npcs == 0 ? 1 : request.npcs;
    std::set<uint64_t> candidate_presses;
    for (const auto& item : wanted_targets){
        int64_t lowest_press = std::max(
            press_for_advance(item.first, highest_span, request.bias),
            (int64_t)request.first_press
        );
        int64_t highest_press = std::min(
            press_for_advance(item.first, lowest_span, request.bias),
            last_press
        );
        for (int64_t press = lowest_press; press <= highest_press; press++){
            if (press % press_step != 0){
                continue;
            }
            candidate_presses.insert((uint64_t)press);
        }
    }

    double seconds_per_advance = request.tick_seconds / (double)press_step;
    bool found = false;
    double best_seconds = 0;
    for (uint64_t press : candidate_presses){
        double press_at_seconds = (double)(press - request.first_press) * seconds_per_advance;
        if (found && press_at_seconds >= best_seconds){
            break;
        }
        for (const PressSchedule& option : schedule_presses(
            request.state, request.timeline, request.timings, press, request.max_schedules
        )){
            auto hit = wanted_targets.find(aimed_advance(press, option.span, request.bias));
            if (hit == wanted_targets.end()){
                continue;
            }
            if (!found || press_at_seconds + option.confirm_seconds < best_seconds){
                found = true;
                best_seconds = press_at_seconds + option.confirm_seconds;
                result.press_advance = press;
                result.schedule = option;
                result.target = hit->second;
            }
            //  the first hit is the best this press has to offer
            break;
        }
    }
    if (!found){
        //  Around one advance in fifty is unreachable due to rapid blinks
        result.failure_reason = std::to_string(wanted_targets.size())
            + " matching advance(s) in range, but none reachable";
        return result;
    }

    result.target_advance = aimed_advance(
        result.press_advance, result.schedule.span, request.bias
    );
    result.success = true;
    return result;
}




}
}
}
