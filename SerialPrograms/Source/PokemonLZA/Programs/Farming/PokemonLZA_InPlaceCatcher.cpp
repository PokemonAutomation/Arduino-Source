/*  In-Place Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceSession.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_DayNightChangeDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_AlertEyeDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA/Programs/PokemonLZA_TrainerBattle.h"
#include "PokemonLZA_InPlaceCatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace std::chrono;



InPlaceCatcher_Descriptor::InPlaceCatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:InPlaceCatcher",
        Pokemon::STRING_POKEMON + " LZA", "In-Place Catcher",
        "Programs/PokemonLZA/InPlaceCatcher.html",
        "Stand in one spot and catch everything that spawns nearby. "
        "Useful for filling up your boxes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{
}

class InPlaceCatcher_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : balls_thrown(m_stats["Balls Thrown"])
        , attacks_fired(m_stats["Attacks Launched"])
        , day_changes(m_stats["Day/Night Changes"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Balls Thrown");
        m_display_order.emplace_back("Attacks Launched");
        m_display_order.emplace_back("Day/Night Changes");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& balls_thrown;
    std::atomic<uint64_t>& attacks_fired;
    std::atomic<uint64_t>& day_changes;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> InPlaceCatcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



InPlaceCatcher::InPlaceCatcher()
    : MAX_BALLS(
        "<b>Max Balls:</b><br>Maximum number of balls to throw before stopping.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 1, 999
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MAX_BALLS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void InPlaceCatcher::day_night_handler(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    InPlaceCatcher_Descriptor::Stats& stats = env.current_stats<InPlaceCatcher_Descriptor::Stats>();

    env.log("Detected day/night change.", COLOR_BLUE);
    stats.day_changes++;
    env.update_stats();

    OverworldPartySelectionWatcher overworld(COLOR_RED, &env.console.overlay());
    int ret = wait_until(
        env.console, context,
        120s,
        {overworld}
    );
    if (ret == 0){
        return;
    }

    stats.errors++;
    env.update_stats();
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to detect end of day/night change after 2 minutes.",
        env.console
    );
}

void InPlaceCatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    InPlaceCatcher_Descriptor::Stats& stats = env.current_stats<InPlaceCatcher_Descriptor::Stats>();

    AlertEyeTracker attack_tracker(COLOR_BLUE, &env.console.overlay(), 5000ms);

    InferenceSession session(
        context, env.console,
        {attack_tracker}
    );

    while (stats.balls_thrown.load(std::memory_order_relaxed) < MAX_BALLS){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        MoveEffectivenessSymbolWatcher battle_menu(COLOR_RED, &env.console.overlay(), 100ms);
        DayNightChangeWatcher day_night(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                while (true){
                    for (int c = 0; c < 10; c++){
                        ssf_press_right_joystick(context, 64, 128, 0ms, 1000ms, 0ms);
                        ssf_press_dpad(context, DpadPosition::DPAD_UP, 0ms, 160ms, 160ms);
                        pbf_mash_button(context, BUTTON_ZL, 1000ms);
                        pbf_press_button(context, BUTTON_ZL, 500ms, 0ms);
                    }
                    pbf_press_button(context, BUTTON_L, 160ms, 160ms);
                }
            },
            {
                battle_menu,
                day_night,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            break;
        case 1:{
            day_night_handler(env, context);
            continue;
        }
        }

        if (!attack_tracker.currently_active()){
            env.log("Detected battle menu. Throwing ball...", COLOR_BLUE);
            pbf_press_button(
                context,
                BUTTON_ZL | BUTTON_ZR,
                500ms, 3500ms
            );
            pbf_press_button(context, BUTTON_L, 160ms, 160ms);
            stats.balls_thrown++;
            env.update_stats();
            continue;
        }

        env.log("Detected battle menu while under attack! Launching attack...", COLOR_ORANGE);
        context.wait_for(500ms);

        TrainerBattleState state;
        if (state.attempt_one_attack(env, env.console, context, true, false, false)){
            stats.attacks_fired++;
            env.update_stats();
            pbf_wait(context, 2000ms);
        }else{
            env.log("Unable to lock-on for attack.", COLOR_RED);
        }

    }


    pbf_wait(context, 5 * TICKS_PER_SECOND);
    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}








}
}
}
