/*  Isolated Hunt Prototype
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Field question: can stationary Let's Go clearing acquire a refused target
 *  with ZR? Start at a prepared anchor, or scout the existing Chargestone court.
 *  Stop after a battle, an unresolved refusal, or bounded empty passes so the
 *  operator can inspect positioning before trying any spawn-reset movement.
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "VideoGames/PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "VideoGames/PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "VideoGames/PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h"
#include "VideoGames/PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "VideoGames/PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_Terarium.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV_LetsGoTools.h"
#include "PokemonSV_IsolatedHuntPrototype.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

IsolatedHuntPrototype_Descriptor::IsolatedHuntPrototype_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:IsolatedHuntPrototype",
        STRING_POKEMON + " SV", "Isolated Hunt Prototype",
        "", "Short supervised clearing and battle-acquisition experiment. Prepare sandwich powers and a healthy lead before starting.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct IsolatedHuntPrototype_Descriptor::Stats : public LetsGoEncounterBotStats{
    Stats()
        : passes(m_stats["Clearing Passes"])
        , refusals(m_stats["Refusals"])
    {
        m_display_order.emplace_back("Clearing Passes");
        m_display_order.emplace_back("Refusals");
    }
    std::atomic<uint64_t>& passes;
    std::atomic<uint64_t>& refusals;
};

std::unique_ptr<StatsTracker> IsolatedHuntPrototype_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

IsolatedHuntPrototype::IsolatedHuntPrototype()
    : LANGUAGE("<b>Game Language:</b>", IV_READER().languages(), LockMode::LOCK_WHILE_RUNNING, true)
    , START(
        "<b>Start:</b><br>Prepared spot: stand on foot, facing the target patch. "
        "Central Plaza: start as if just flown there; uses the existing Chargestone court approach. "
        "Chespin requires the Canyon diversity upgrade and a verified Grass spawn patch.",
        {
            {Start::PREPARED_SPOT, "prepared", "Prepared hunting spot (Chespin or Charcadet)"},
            {Start::CENTRAL_PLAZA, "central-plaza", "Central Plaza to Chargestone court"},
        },
        LockMode::LOCK_WHILE_RUNNING, Start::PREPARED_SPOT
    )
    , RUN_SECONDS(
        "<b>Clearing time limit (seconds):</b><br>Starts after the approach. Stops between passes; each pass takes at most about 17 seconds.",
        LockMode::LOCK_WHILE_RUNNING, 120, 15, 600
    )
    , EMPTY_PASSES(
        "<b>Stop after consecutive passes without a kill:</b><br>Stops for inspection without resetting nearby spawns.",
        LockMode::LOCK_WHILE_RUNNING, 3, 1, 10
    )
    , NOTIFICATIONS({
        &ENCOUNTER_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(START);
    PA_ADD_OPTION(RUN_SECONDS);
    PA_ADD_OPTION(EMPTY_PASSES);
    PA_ADD_OPTION(ENCOUNTER_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void IsolatedHuntPrototype::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    auto& stats = env.current_stats<IsolatedHuntPrototype_Descriptor::Stats>();
    assert_16_9_720p_min(env.logger(), env.console);
    require_player(env.console, context, BUTTON_LCLICK);

    OverworldWatcher overworld(env.logger(), COLOR_BLUE);
    if (wait_until(env.console, context, 5000ms, {overworld}) < 0){
        throw UserSetupError(env.logger(), "Start in the overworld at the selected anchor.");
    }

    OverworldSensors sensors(env.logger(), env.console, context);
    OverworldBattleTracker battle_tracker(env.logger(), sensors);
    LetsGoEncounterBotTracker encounters(env, env.console, stats, sensors.lets_go_kill);
    std::string stop_reason;
    NormalBattleMenuWatcher battle_menu(COLOR_RED);
    int result = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            if (START == Start::CENTRAL_PLAZA){
                env.console.log("Prototype state: approach Chargestone court.");
                central_to_chargestone(env.program_info(), env.console, context);
                pbf_wait(context, 3000ms);
                // Match the existing BBQ court approach; its Chespin footprint still needs field verification.
                pbf_press_button(context, BUTTON_L, 160ms, 400ms);
                pbf_move_left_joystick(context, {0, +1}, 2000ms, 400ms);
                pbf_move_left_joystick(context, {+1, 0}, 1440ms, 400ms);
                pbf_press_button(context, BUTTON_L, 160ms, 400ms);
                context.wait_for_all_requests();
            }

            WallClock deadline = current_time() + std::chrono::seconds(RUN_SECONDS);
            uint8_t empty = 0;
            while (current_time() < deadline){
                const uint64_t before = stats.m_kills.load();
                env.console.log("Prototype state: clearing pass " + std::to_string(stats.passes.load() + 1));
                SweatBubbleWatcher bubble(COLOR_GREEN);
                int refused = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        pbf_press_button(context, BUTTON_R, 160ms, 1600ms);
                    },
                    {bubble}
                );
                if (refused == 0){
                    stats.refusals++;
                    env.update_stats();
                    env.console.log("Prototype state: refusal detected; trying ZR. Refusal does not confirm a shiny.");
                    pbf_mash_button(context, BUTTON_ZR, 5000ms);
                    pbf_wait(context, 10000ms);
                    context.wait_for_all_requests();
                    stop_reason = "Refusal acquisition timed out without a battle. Inspect the target and positioning.";
                    return;
                }

                // A fixed observation window keeps a long kill chain from extending a pass indefinitely.
                pbf_wait(context, 12000ms);
                context.wait_for_all_requests();
                sensors.throw_if_no_sound();
                stats.passes++;
                uint64_t kills = stats.m_kills.load() - before;
                empty = kills == 0 ? empty + 1 : 0;
                env.update_stats();
                env.console.log("Prototype pass: kills=" + std::to_string(kills) +
                    ", consecutive empty passes=" + std::to_string(empty));
                if (empty >= EMPTY_PASSES){
                    stop_reason = "No clearing progress. Inspect spawn density, lead health and target reachability.";
                    return;
                }
            }
            stop_reason = "Clearing time limit reached.";
        },
        {battle_menu}
    );

    if (result == 0){
        env.console.log("Prototype state: battle detected; using shared encounter/shiny handling.");
        sensors.throw_if_no_sound();
        stats.m_encounters++;
        env.update_stats();
        encounters.encounter_rate_tracker().report_encounter();
        bool caught = false, should_save = false;
        try{
            process_battle(caught, should_save, env, ENCOUNTER_OPTIONS, env.console, context,
                battle_tracker, encounters.encounter_frequencies(), stats.m_shinies, LANGUAGE);
        }catch (ProgramFinishedException&){
            env.console.log("Prototype stopped by encounter policy. Leaving the battle for inspection.", COLOR_BLUE);
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
            return;
        }
        if (should_save){
            save_game_from_overworld(env.program_info(), env.console, context);
        }
        // Do not resume a timed movement path after an encounter changes the anchor.
        stop_reason = "Battle handled. Inspect positioning before another run.";
    }
    env.console.log("Prototype stopped: " + stop_reason, COLOR_BLUE);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
