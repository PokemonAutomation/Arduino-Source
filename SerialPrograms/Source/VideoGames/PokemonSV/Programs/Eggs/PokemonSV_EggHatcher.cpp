/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "VideoGames/PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "VideoGames/PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "VideoGames/PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "VideoGames/PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:EggHatcher",
        STRING_POKEMON + " SV", "Egg Hatcher",
        "Programs/PokemonSV/EggHatcher.html",
        "Automatically hatch eggs from boxes.",
        PanelQualityStanding::OFFICIAL,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct EggHatcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_hatched(m_stats["Hatched"])
        , m_shinies(m_stats["Shinies"])
        , m_released(m_stats["Released"])
        , m_attacked(m_stats["Attacked"])
        , m_errors(m_stats["Errors"])
        , m_hatch_time_ms(m_stats["Hatch Time (ms)"])
    {
        m_display_order.emplace_back("Hatched");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Released", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Attacked", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Hatch Time (ms)", ALWAYS_HIDDEN);
    }
    virtual std::string to_str(PrintMode mode) const override{
        std::string str = StatsTracker::to_str(mode);
        if (mode == DISPLAY_ON_SCREEN){
            str += avg_time_per_egg_text(m_hatch_time_ms.load(), m_hatched.load());
        }
        return str;
    }
    std::atomic<uint64_t>& m_hatched;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_released;
    std::atomic<uint64_t>& m_attacked;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_hatch_time_ms;
};
std::unique_ptr<StatsTracker> EggHatcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggHatcher::EggHatcher()
    : GO_HOME_WHEN_DONE(false)
    , START_LOCATION(
        "<b>Start location:</b><br>Where to start the hatcher program.<br>"
        "Zero Gate Flying Spot: Stand at Zero Gate flying spot. The flying spot is already unlocked.<br>"
        "Anywhere safe, on ride: You are in a safe location with no wild encounters or NPCs. You are on your ride legendary.<br>"
        "Anywhere safe, on foot: You are in a safe location with no wild encounters or NPCs. You stand on foot.<br>",
        {
            {StartLocation::ZeroGateFlyingSpot, "zero-gate", "Zero Gate Flying Spot"},
            {StartLocation::AnywhereOnRide, "anywhere-on-ride", "Anywhere safe, on ride."},
            {StartLocation::AnywhereOffRide, "anywhere-off-ride", "Anywhere safe, on foot."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartLocation::ZeroGateFlyingSpot
    )
    , BOXES(
        "<b>How many boxes of eggs to hatch:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 1, 32
    )
    , m_battle_ai_description(
        "<b>Battle AI if Attacked:</b><br>"
        "Occasionally (especially on the Switch 2), you will get attacked by wild " + STRING_POKEMON + ". "
        "If the following section is enabled, it attempts to recover by killing the wild " + STRING_POKEMON + ". "
        "If disabled, the program will instead reset the game. "
        "This table applies to the egg hatching " + STRING_POKEMON + " (the Flame Body one)."
    )
    , BATTLE_AI(false, GroupOption::EnableMode::DEFAULT_ENABLED, 1)
    , HAS_CLONE_RIDE_POKEMON(
        "<b>Cloned Ride Legendary 2nd in Party:</b><br>"
        "Ride legendary cannot be cloned after patch 1.0.1. To preserve the existing clone while hatching eggs, "
        "place it as second in party before starting the program. "
        "The program will skip the first row of eggs in the box as a result.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , RELEASE_NON_SHINY(
        "<b>Release Non-Shiny Hatchlings:</b><br>"
        "After each column hatches, check every new hatchling and <b>release</b> it if it is not shiny. "
        "Shiny hatchlings are put back in the box. Releasing cannot be undone, so only turn this on if you "
        "only want shinies. Your Flame Body " + STRING_POKEMON + " (and cloned ride legendary) are never released.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(BOXES);
    PA_ADD_OPTION(m_battle_ai_description);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(HAS_CLONE_RIDE_POKEMON);
    PA_ADD_OPTION(RELEASE_NON_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void EggHatcher::hatch_one_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();

    for (uint8_t column_index = 0; column_index < 6; column_index++){
        uint8_t num_eggs = 0, num_non_egg_pokemon = 0;
        {
            const uint8_t expected_empty_slots_in_party = HAS_CLONE_RIDE_POKEMON ? 4 : 5;
            if (check_empty_slots_in_party(env.program_info(), env.console, context) != expected_empty_slots_in_party){
                throw_and_log<FatalProgramException>(
                    env.console, ErrorReportMode::SEND_ERROR_REPORT,
                    "Your party should have " + std::to_string(6 - expected_empty_slots_in_party) + " " + STRING_POKEMON +
                    " and " + std::to_string(expected_empty_slots_in_party) + " empty slots.",
                    env.console
                );
            }
        }

        load_one_column_to_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, column_index, HAS_CLONE_RIDE_POKEMON);
        // Move cursor to party lead so that we can examine rest of party to detect eggs.
        move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);

        std::tie(num_eggs, num_non_egg_pokemon) = check_egg_party_column(env.program_info(), env.console, context);
        if (num_eggs == 0){
            if (num_non_egg_pokemon == 0){
                // nothing in this column
                env.log("Nothing in column " + std::to_string(column_index+1) + ".");
                env.console.overlay().add_log("Empty column");
                continue;
            }

            // we have only non-egg pokemon in the column
            // Move them back
            env.log("Only non-egg pokemon in column, move them back.");
            env.console.overlay().add_log("No egg in column");
            unload_one_column_from_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, column_index, HAS_CLONE_RIDE_POKEMON);
            continue;
        }
        
        env.log("Loaded " + std::to_string(num_eggs) + " eggs to party.");
        // Only auto-release when the column was all eggs. If the column also had already-hatched
        // Pokemon mixed in, we can't tell which party slots are the new hatchlings, so leave it alone.
        const uint8_t num_hatched = num_eggs;
        const bool column_was_all_eggs = num_non_egg_pokemon == (HAS_CLONE_RIDE_POKEMON ? 1 : 0);
        env.console.overlay().add_log("Load " + std::to_string(num_eggs) + " eggs");
        leave_box_system_to_overworld(env.program_info(), env.console, context);

        auto hatched_callback = [&](uint8_t){  
            add_hatch_time(m_last_hatch_time, stats.m_hatch_time_ms);
            stats.m_hatched++;
            env.update_stats();
        };

        switch (START_LOCATION){
        case StartLocation::ZeroGateFlyingSpot:
            hatch_eggs_at_zero_gate(
                env,
                env.console,
                context,
                &NOTIFICATION_ERROR_RECOVERABLE,
                &BATTLE_AI,
                &stats.m_attacked,
                num_eggs,
                hatched_callback
            );
            reset_position_at_zero_gate(env.program_info(), env.console, context);
            break;
        case StartLocation::AnywhereOnRide:
        case StartLocation::AnywhereOffRide: // the program already pressed + to get on ride at start
        {
            const bool on_ride = true;
            hatch_eggs_anywhere(env, env.console, context, on_ride, num_eggs, hatched_callback);
            break;
        }
        default:
            throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Unknown StartLocation");
        }

        enter_box_system_from_overworld(env.program_info(), env.console, context);

        num_eggs = check_egg_party_column(env.program_info(), env.console, context).first;
        if (num_eggs > 0){
            throw_and_log<FatalProgramException>(
                env.console, ErrorReportMode::SEND_ERROR_REPORT,
                "Detected egg in party after hatching.",
                env.console
            );
        }

        if (RELEASE_NON_SHINY){
            if (column_was_all_eggs){
                release_non_shiny_hatchlings(env, context, num_hatched);
            }else{
                env.log("Column had non-egg Pokemon mixed with eggs. Skipping release for this column to be safe.", COLOR_RED);
                env.console.overlay().add_log("Mixed column: no release", COLOR_YELLOW);
            }
        }

        unload_one_column_from_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, column_index, HAS_CLONE_RIDE_POKEMON);
    }

    context.wait_for_all_requests();
}

void EggHatcher::release_non_shiny_hatchlings(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t num_hatched){
    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();

    // Party: Flame Body lead, (cloned ride legendary), then the hatchlings.
    // Go from the bottom of the party up, so releasing one never shifts the ones we still need to check.
    const uint8_t first_hatchling_row = HAS_CLONE_RIDE_POKEMON ? 2 : 1;
    for (uint8_t i = 0; i < num_hatched; i++){
        const uint8_t party_row = (uint8_t)(first_hatchling_row + num_hatched - 1 - i);
        move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, party_row, 0);
        context.wait_for_all_requests();

        // Shiny star shows on the stats/judge view of the box.
        change_view_to_stats_or_judge(env.console, context);
        VideoOverlaySet overlay_set(env.console.overlay());
        BoxShinyWatcher shiny_detector;
        shiny_detector.make_overlays(overlay_set);
        const bool shiny = wait_until(
            env.console, context,
            std::chrono::milliseconds(500),
            {shiny_detector}
        ) == 0;

        if (shiny){
            env.log("Party slot " + std::to_string(party_row) + ": shiny! Keeping it.", COLOR_BLUE);
            env.console.overlay().add_log("Shiny! Keep", COLOR_GREEN);
            stats.m_shinies++;
            env.update_stats();
            continue;
        }

        env.log("Party slot " + std::to_string(party_row) + ": not shiny. Releasing.");
        env.console.overlay().add_log("Not shiny: release", COLOR_WHITE);
        size_t local_errors = 0;
        release_one_pokemon(env.program_info(), env.console, context, local_errors);
        stats.m_released++;
        stats.m_errors += local_errors;
        env.update_stats();
    }
}

void EggHatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();

    //  Connect the controller.
    require_player(env.console, context, BUTTON_L);
    m_last_hatch_time = std::chrono::steady_clock::now();

    if (START_LOCATION == StartLocation::AnywhereOffRide){
        // Get on ride:
        pbf_press_button(context, BUTTON_PLUS, 400ms, 800ms);
        context.wait_for_all_requests();
    }

    try{
        enter_box_system_from_overworld(env.program_info(), env.console, context);
        // // Wait one second to let game load box UI
        // context.wait_for(std::chrono::seconds(1));

        for (uint8_t i = 0; i < BOXES; i++){
            if (i > 0){
                env.log("Go to next box.");
                env.console.overlay().add_log("Next box", COLOR_WHITE);
                move_to_right_box(context);
            }
            context.wait_for_all_requests();

            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

            hatch_one_box(env, context);
        }
    }catch (Exception&){
        stats.m_errors++;
        env.update_stats();
        throw;
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
