/*  Post MMO Spawn Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_PostMMOSpawnReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



PostMMOSpawnReset_Descriptor::PostMMOSpawnReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:PostMMOSpawnReset",
        STRING_POKEMON + " LA", "Post-MMO Spawn Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/PostMMOSpawnReset.md",
        "Constantly reset the spawn after MMO finishes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class PostMMOSpawnReset_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> PostMMOSpawnReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


PostMMOSpawnReset::PostMMOSpawnReset()
    : TURN_DURATION0(
        "<b>Camera Turn:</b><br>How many ticks to turn the camera. <br>Positive values for right turns. Negative values for left turns.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 ms"
    )
    , FORWARD_DURATION0(
        "<b>Move Forward:</b><br>After turning the camera, how many ticks to move forward.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 ms"
    )
    , WAIT_DURATION0(
        "<b>Wait Time:</b><br> Wait time after movement.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , SHINY_DETECTED("Shiny Detected Action", "", "0 ms")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TURN_DURATION0);
    PA_ADD_OPTION(FORWARD_DURATION0);
    PA_ADD_OPTION(WAIT_DURATION0);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void PostMMOSpawnReset::run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PostMMOSpawnReset_Descriptor::Stats& stats = env.current_stats<PostMMOSpawnReset_Descriptor::Stats>();

    // From game to Switch Home
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
    {
        float shiny_coefficient = 1.0;
        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            return on_shiny_callback(env, env.console, SHINY_DETECTED, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [this, &env](ProControllerContext& context){
                reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
                env.console.log("Entered game! Checking shiny sound...");

                // forward portion
                if (TURN_DURATION0.get() > 0ms){
                    pbf_move_right_joystick(context, 255, 128, TURN_DURATION0, 0ms);
                }else if (TURN_DURATION0.get() < 0ms){
                    pbf_move_right_joystick(context, 0, 128, -TURN_DURATION0.get(), 0ms);
                }
                
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, FORWARD_DURATION0);

                pbf_wait(context, WAIT_DURATION0);

                context.wait_for_all_requests();
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0){
            on_shiny_sound(env, env.console, context, SHINY_DETECTED, shiny_coefficient);
        }
    }

    stats.attempts++;
    env.console.log("No shiny detected, restarting the game!");
}


void PostMMOSpawnReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PostMMOSpawnReset_Descriptor::Stats& stats = env.current_stats<PostMMOSpawnReset_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            // run_iteration() restarts the game first then listens to shiny sound.
            // If there is any error generated when the game is running and is caught here,
            // we just do nothing to handle the error as in the next iteration of run_iteration()
            // the game will be immediately restarted.
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
