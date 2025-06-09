/*  Max Lair Run Adventure
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Start.h"
#include "PokemonSwSh_MaxLair_Run_Adventure.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


AdventureResult run_adventure(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    size_t boss_slot,
    AdventureRuntime& runtime
){
    env.log("Starting new adventure...");
    env.update_stats();

    send_status_notification(env, runtime);

    std::shared_ptr<const ImageRGB32> entrance[4];
    GlobalStateTracker state_tracker(scope, env.consoles.size());

    if (!start_adventure(
        env, scope,
        state_tracker,
        entrance,
        env.consoles[runtime.host_index], boss_slot,
        runtime.hosting_settings,
        runtime.path_stats,
        runtime.session_stats,
        runtime.consoles
    )){
        runtime.session_stats.add_error();
        return AdventureResult::START_ERROR;
    }

    uint64_t epoch = 0;
    SpinLock lock;

    std::atomic<bool> stop(false);
    std::atomic<bool> error(false);

    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        StateMachineAction action;
        while (true){
            //  Dump current state, but don't spam if nothing has changed.
            {
                WriteSpinLock lg(lock);
                std::pair<uint64_t, std::string> status = state_tracker.dump();
                if (status.first > epoch){
                    console.log("State Tracker\n" + status.second);
                    epoch = status.first;
                }
            }

            size_t index = console.index();
            action = run_state_iteration(
                runtime, index,
                env, console, context, boss_slot != 0 && console.index() == runtime.host_index,
                state_tracker, runtime.actions,
                *entrance[index]
            );
            switch (action){
            case StateMachineAction::KEEP_GOING:
                continue;
            case StateMachineAction::DONE_WITH_ADVENTURE:
                env.log("End of adventure.", COLOR_PURPLE);
                return;
            case StateMachineAction::STOP_PROGRAM:
                env.log("End of adventure. Stop program requested...", COLOR_PURPLE);
                if (runtime.go_home_when_done){
                    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
                }
                stop.store(true, std::memory_order_release);
                return;
            case StateMachineAction::RESET_RECOVER:
                env.log("Error detected. Attempting to correct by resetting...", COLOR_RED);
                runtime.session_stats.add_error();
                error.store(true, std::memory_order_release);
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
                reset_game_from_home_with_inference(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
                return;
            }
        }
    });

    std::string boss = state_tracker.infer_actual_state(0).boss;
    if (!boss.empty()){
        runtime.last_boss = std::move(boss);
    }

    if (stop.load(std::memory_order_acquire)){
        return AdventureResult::STOP_PROGRAM;
    }
//    if (error.load(std::memory_order_acquire)){
//        return AdventureResult::START_ERROR;
//    }
    return AdventureResult::FINISHED;
}





void loop_adventures(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const Consoles& consoles,
    size_t host_index, size_t boss_slot,
    const EndBattleDecider& decider,
    bool go_home_when_done,
    HostingSettings& HOSTING,
    TouchDateIntervalOption& TOUCH_DATE_INTERVAL,
    EventNotificationOption& notification_status,
    EventNotificationOption& notification_shiny
){
    Stats& stats = env.current_stats<Stats>();

    AdventureRuntime runtime(
        env.consoles,
        host_index,
        consoles,
        decider,
        go_home_when_done,
        HOSTING,
        notification_status,
        notification_shiny,
        stats
    );

    size_t restart_count = 0;
    while (true){
        //  Touch the date.
        if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                env.log("Touching date to prevent rollover.");
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
                touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
                resume_game_back_out(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 5 * TICKS_PER_SECOND);
            });
        }

        AdventureResult result = run_adventure(env, scope, boss_slot, runtime);
        switch (result){
        case AdventureResult::FINISHED:
            restart_count = 0;
            continue;
        case AdventureResult::STOP_PROGRAM:
            return;
        case AdventureResult::START_ERROR:
            restart_count++;
            if (restart_count == 3){
                report_error(
                    &env.logger(),
                    env.program_info(),
                    "Error",
                    {{"Message:", "Failed to start adventure 3 times in the row."}}
                );
                throw_and_log<OperationFailedException>(
                    env.logger(), ErrorReport::SEND_ERROR_REPORT,
                    "Failed to start adventure 3 times in the row."
                );
            }
            env.log("Failed to start adventure. Resetting all Switches...", COLOR_RED);
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
                reset_game_from_home_with_inference(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            });
            continue;
        }
    }
}






















}
}
}
}
