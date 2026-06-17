/*  RNG Loop Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/Time.h"
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_RngLoopRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace {
    // pull one side out of each (held, pickup) pair, sorted and de-duplicated
    std::vector<AdvRngState> states_from_pairs(
        const std::vector<std::pair<AdvRngState, AdvRngState>>& pairs,
        bool take_first
    ){
        std::vector<AdvRngState> states;
        states.reserve(pairs.size());
        for (const auto& pair : pairs){
            states.emplace_back(take_first ? pair.first : pair.second);
        }
        std::sort(states.begin(), states.end());
        states.erase(std::unique(states.begin(), states.end()), states.end());
        return states;
    }
} // namespace


void select_calibrations_for_frame(
    SingleSwitchProgramEnvironment& env,
    EggCalibrationHistories& hist,
    const std::vector<uint16_t>& seed_values,
    const int16_t& seed_position,
    uint64_t advances,
    bool pickup_frame,
    bool previously_hit_held_frame,
    uint16_t times_not_held
){
    RngCalibrations& calibrations = hist.calibrations;
    const RngCalibrationHistory& egg_history = hist.frame(pickup_frame);
    const RngCalibrationHistory& wild_history = hist.wild;
    const RngUncertainHistory& egg_uncertain_history = hist.egg_uncertain;

    if (egg_history.results.size() > 0){
        calibrations = get_calibrations(env.console, egg_history, seed_values, seed_position, advances, !pickup_frame);
    }
    if (wild_history.results.size() > egg_history.results.size()){
        calibrations.seed_offset = get_seed_calibration_frames(wild_history, seed_values, seed_position);
        env.log("Updated Seed Calibration (frames): " + std::to_string(calibrations.seed_offset));
    }

    // if previous resets had uncertain advances, slightly modify the seed delay to try to hit a different target
    if (pickup_frame){
        apply_seed_bump(calibrations, egg_uncertain_history);
    }

    // if the egg isn't ready over several resets,
    // or if there is repeated uncertainty about advances from previous resets,
    // bump the advances
    if (!pickup_frame && !previously_hit_held_frame){
        if (egg_history.results.empty()){
            // avoid cumulative bumps when there is no history
            if (egg_uncertain_history.calibrations.size() > 0){
                calibrations = egg_uncertain_history.calibrations.back();
            }else{
                env.log("No calibration history yet. Using initial calibrations from UI...");
                calibrations = hist.initial_calibrations;
            }
        }
        double bumpval = std::floor(times_not_held / 4) + std::floor(egg_uncertain_history.results.size() / 2);
        double advances_bump = std::pow(-1, bumpval) * std::floor((bumpval+1) / 2); // 0, -1, +1, -2, +2...
        double orig_csf_offset = calibrations.csf_offset;
        calibrations.csf_offset = fmod(orig_csf_offset + advances_bump, 2);
        calibrations.ingame_offset += advances_bump - (calibrations.csf_offset - orig_csf_offset);
    }
}


WildCatchOutcome catch_wild_for_seed_id(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    Language language,
    const WallClock& timestamp,
    uint64_t advances,
    int sweet_scent_from_last,
    uint64_t& balls_left,
    const std::set<std::string>& species_list,
    const RngStatsDatabase& stats_data,
    AdvObservedPokemon& pokemon,
    RngStats& species_stats,
    AdvRngFilters& filters,
    uint64_t& wild_advances_estimate,
    const std::function<void(const AdvRngFilters&)>& publish_filters
){
    uint32_t rng_wait = 50 * random_u32(0, 20); // avoid hitting the same wild targets
    pbf_wait(context, std::chrono::milliseconds(rng_wait));

    use_sweet_scent_from_overworld(env.console, context, sweet_scent_from_last);

    WallDuration elapsed = current_time() - timestamp;
    auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);
    wild_advances_estimate = uint64_t(advances + 2 * elapsed_ms.count() / FRLG_FRAME_DURATION);
    env.log("Wild advances estimate: " + std::to_string(wild_advances_estimate));

    int ret = watch_for_shiny_encounter(env.console, context);
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "catch_wild_for_seed_id(): Failed to trigger battle",
            env.console
        );
    }
    if (ret == 1){
        return WildCatchOutcome::shiny;
    }

    int balls_thrown = auto_catch(env.console, context, balls_left);
    if (balls_thrown < 0){
        errors++;
        send_program_recoverable_error_notification(
            env, error_notification,
            "auto_catch() encountered an error."
        );
        return WildCatchOutcome::failed;
    }else if(balls_thrown == 0){
        env.log("Failed catch.");
        return WildCatchOutcome::failed;
    }

    balls_left -= balls_thrown;

    go_to_summary(env.console, context);
    pokemon = read_summary(env.console, context, language, species_list);
    try{
        species_stats = stats_data.get_throw(pokemon.species);
    }catch (const InternalProgramError& err){
        env.log(err.message());
        env.log("Failed to load base stats.");
        return WildCatchOutcome::failed;
    }

    filters = observation_to_filters(pokemon, species_stats.base_stats, AdvRngMethod::Any);
    publish_filters(filters);
    return WildCatchOutcome::caught;
}


bool confirm_held_frame_hit(
    RngCalibrationHistory& calibration_history,
    const RngUncertainHistory& egg_uncertain_history,
    bool locked_in,
    uint16_t target_seed,
    uint64_t target_advances
){
    if (locked_in){
        return have_hit_target(target_seed, target_advances, calibration_history.results.back());
    }
    // an uncertain history means we can't yet trust the committed results
    if (egg_uncertain_history.results.size() != 0){
        return false;
    }

    // keep only the calibrations that resulted in hitting the target
    bool confirmed = false;
    std::vector<AdvRngState> kept_results;
    std::vector<RngCalibrations> kept_calibrations;
    for (size_t i = 0; i < calibration_history.results.size(); i++){
        if (have_hit_target(target_seed, target_advances, calibration_history.results[i])){
            confirmed = true;
            kept_results.emplace_back(calibration_history.results[i]);
            kept_calibrations.emplace_back(calibration_history.calibrations[i]);
        }
    }
    if (confirmed){
        calibration_history.results = kept_results;
        calibration_history.calibrations = kept_calibrations;
    }
    return confirmed;
}


std::vector<AdvRngState> refine_calibration_with_rare_candy(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history,
    const RngCalibrations& calibrations,
    uint16_t max_history_length,
    uint64_t max_rare_candies,
    AdvRngMethod method,
    bool safari_zone,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    const std::function<std::vector<AdvRngState>(AdvRngFilters&)>& run_search,
    const std::function<void(const std::vector<AdvRngState>&)>& publish_hits,
    const std::function<void(const AdvRngFilters&)>& publish_filters,
    const std::function<bool(const std::vector<AdvRngState>&)>& result_determined
){
    std::vector<AdvRngState> search_hits = run_search(filters);
    publish_hits(search_hits);

    bool force_finish = (max_rare_candies == 0) || result_determined(search_hits);
    bool finished = update_history(
        env.console, uncertain_history, calibration_history, max_history_length,
        calibrations, search_hits, 1, 2, force_finish
    );

    for (uint64_t i = 0; i < max_rare_candies; i++){
        if (finished){
            break;
        }

        bool failed = use_rare_candy(
            env.console, context, language, pokemon, filters,
            base_stats, method, safari_zone, i == 0
        );
        if (failed){
            update_history(
                env.console, uncertain_history, calibration_history,
                max_history_length, calibrations, search_hits, 1, 2, true
            );
            errors++;
            send_program_recoverable_error_notification(
                env, error_notification,
                "Failed to use Rare Candy."
            );
        }
        publish_filters(filters);

        search_hits = run_search(filters);
        publish_hits(search_hits);

        force_finish = (
               failed
            || (i == (max_rare_candies - 1))
            || result_determined(search_hits)
        );
        finished = update_history(
            env.console, uncertain_history,
            calibration_history, max_history_length,
            calibrations, search_hits,
            1, 2, force_finish
        );
    }

    return search_hits;
}


std::vector<AdvRngState> refine_egg_calibration(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    AdvObservedPokemon& observed_egg,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history,
    const RngCalibrations& calibrations,
    uint16_t max_history_length,
    uint64_t candies_left,
    const EggRefineOptions& options,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    const std::function<std::vector<std::pair<AdvRngState, AdvRngState>>(AdvRngFilters&)>& run_search,
    const std::function<void(const std::vector<AdvRngState>&)>& publish_hits,
    const std::function<void(const AdvRngFilters&)>& publish_filters,
    const std::function<bool(const std::vector<std::pair<AdvRngState, AdvRngState>>&)>& indistinguishable
){
    std::vector<std::pair<AdvRngState, AdvRngState>> search_hits = run_search(filters);
    std::vector<AdvRngState> hits = states_from_pairs(search_hits, options.take_first);
    publish_hits(hits);

    bool force_finish = (candies_left == 0)
        || all_equal(hits)
        || indistinguishable(search_hits);
    bool finished = update_history(
        env.console, uncertain_history, calibration_history, max_history_length,
        calibrations, hits, 1, options.advance_radius, force_finish
    );

    for (uint64_t i = 0; i < candies_left; i++){
        if (finished){
            break;
        }

        bool failed = use_rare_candy(
            env.console, context, language, observed_egg, filters,
            base_stats, AdvRngMethod::Any, false, i == 0, options.from_last
        );
        if (failed){
            errors++;
            send_program_recoverable_error_notification(
                env, error_notification,
                "Failed to use Rare Candy."
            );
        }
        publish_filters(filters);

        search_hits = run_search(filters);
        hits = states_from_pairs(search_hits, options.take_first);
        publish_hits(hits);

        force_finish = failed || (i == (candies_left - 1)) || indistinguishable(search_hits);
        finished = update_history(
            env.console, uncertain_history, calibration_history, max_history_length,
            calibrations, hits, 1, options.advance_radius, force_finish
        );
    }

    return hits;
}

}
}
}
