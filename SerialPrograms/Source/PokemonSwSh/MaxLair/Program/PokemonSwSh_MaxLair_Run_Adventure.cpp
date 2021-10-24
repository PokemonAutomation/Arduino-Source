/*  Max Lair Run Adventure
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Start.h"
#include "PokemonSwSh_MaxLair_Run_Adventure.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


AdventureResult run_adventure(
    MultiSwitchProgramEnvironment& env,
    const QString& program_name,
    size_t host_index, size_t boss_slot,
    const MaxLairConsoleOptions& player0,
    const MaxLairConsoleOptions& player1,
    const MaxLairConsoleOptions& player2,
    const MaxLairConsoleOptions& player3,
    HostingSettings& HOSTING,
    const EndBattleDecider& decider,
    EventNotificationOption& notification_noshiny,
    EventNotificationOption& notification_shiny,
    bool save_path_if_host,
    bool return_to_entrance, bool go_home_when_done
){
    Stats& stats = env.stats<Stats>();

    MaxLairRuntime settings{
        program_name,
        {
            &player0,
            &player1,
            &player2,
            &player3,
        },
        decider,
        notification_noshiny,
        notification_shiny,
        stats
    };

    env.log("Starting new adventure...");
    env.update_stats();

    QImage entrance[4];
    GlobalStateTracker state_tracker(env, env.consoles.size());

    if (!start_raid(
        env,
        state_tracker,
        entrance,
        env.consoles[host_index], boss_slot,
        HOSTING,
        program_name, stats
    )){
        stats.add_error();
        return AdventureResult::ERROR;
    }

    uint64_t epoch = 0;
    SpinLock lock;

    std::atomic<bool> stop(false);
    std::atomic<bool> error(false);

    env.run_in_parallel([&](ConsoleHandle& console){
        StateMachineAction action;
        while (true){
            //  Dump current state, but don't spam if nothing has changed.
            {
                SpinLockGuard lg(lock);
                std::pair<uint64_t, std::string> status = state_tracker.dump();
                if (status.first > epoch){
                    console.log("State Tracker\n" + status.second);
                    epoch = status.first;
                }
            }

            size_t index = console.index();
            action = run_state_iteration(
                settings, index,
                env, console, save_path_if_host && console.index() == host_index,
                state_tracker, decider,
                entrance[index]
            );
            switch (action){
            case StateMachineAction::KEEP_GOING:
                continue;
            case StateMachineAction::DONE_WITH_ADVENTURE:
                env.log("End of adventure.", "purple");
                return;
            case StateMachineAction::STOP_PROGRAM:
                env.log("End of adventure. Stop program requested...", "purple");
                if (go_home_when_done){
                    pbf_press_button(console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
                }
                stop.store(true, std::memory_order_release);
                return;
            case StateMachineAction::RESET_RECOVER:
                env.log("Error detected. Attempting to correct by resetting...", Qt::red);
                stats.add_error();
                error.store(true, std::memory_order_release);
//                QImage screen = console.video().snapshot();
//                dump_image(console, MODULE_NAME, "ResetRecovery", screen);
                if (return_to_entrance){
                    pbf_press_button(console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
                    reset_game_from_home_with_inference(env, console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
                }
                return;
            }
        }
    });

    if (stop.load(std::memory_order_acquire)){
        return AdventureResult::STOP_PROGRAM;
    }
    if (error.load(std::memory_order_acquire)){
        return AdventureResult::ERROR;
    }
    return AdventureResult::FINISHED;
}


}
}
}
}
