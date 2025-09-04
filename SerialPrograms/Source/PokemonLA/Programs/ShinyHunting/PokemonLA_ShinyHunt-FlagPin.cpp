/*  Shiny Hunt - Fixed Point
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
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_FlagNavigationAir.h"
#include "PokemonLA_ShinyHunt-FlagPin.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



ShinyHuntFlagPin_Descriptor::ShinyHuntFlagPin_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-FlagPin",
        STRING_POKEMON + " LA", "Shiny Hunt - Flag Pin",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-FlagPin.md",
        "Repeatedly travel to a flag pin to shiny hunt " + STRING_POKEMON + " around it.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class ShinyHuntFlagPin_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
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
std::unique_ptr<StatsTracker> ShinyHuntFlagPin_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHuntFlagPin::ShinyHuntFlagPin()
    : ENROUTE_DISTANCE(
        "<b>Enroute Distance:</b><br>"
        "You are considered \"enroute\" if you are further than this distance from the flag.<br><br>"
        "<font color=\"red\">If you wish to ignore enroute shinies, scroll down to "
        "\"Enroute Shiny Action\" and set it to ignore shinies. "
        "Keep in mind that the shiny sound radius is 30 and you will need some headroom.</font>",
        LockMode::LOCK_WHILE_RUNNING,
        60
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while enroute to the flag. (defined as being more than the \"Enroute Distance\" specified above)",
        "0 ms"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if a shiny is detected at or near the flag. (defined as being less than the \"Enroute Distance\" specified above)",
        "0 ms"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &SHINY_DETECTED_DESTINATION.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , STOP_DISTANCE(
        "<b>Stop Distance:</b><br>"
        "Reset the game when you come within this distance of the flag. "
        "Don't set this too small. The navigation is not precise enough to land directly on the flag.",
        LockMode::LOCK_WHILE_RUNNING,
        20
    )
    , FLAG_REACHED_DELAY(
        "<b>Target Reached Delay:</b><br>"
        "Once you have reached the flag, wait this many seconds to ensure everything loads and that any shinies are heard before resetting.",
        LockMode::LOCK_WHILE_RUNNING,
        1.0, 0, 60
    )
    , NAVIGATION_TIMEOUT(
        "<b>Navigation Timeout:</b><br>Give up and reset if flag is not reached after this many seconds.",
        LockMode::LOCK_WHILE_RUNNING,
        180, 0
    )
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(ENROUTE_DISTANCE);
    PA_ADD_OPTION(RESET_METHOD);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(STOP_DISTANCE);
    PA_ADD_OPTION(FLAG_REACHED_DELAY);
    PA_ADD_OPTION(NAVIGATION_TIMEOUT);
}



void ShinyHuntFlagPin::run_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool& fresh_from_reset
){
    ShinyHuntFlagPin_Descriptor::Stats& stats = env.current_stats<ShinyHuntFlagPin_Descriptor::Stats>();
    stats.attempts++;

    {
        std::atomic<double> flag_distance(10000);

        float shiny_coefficient = 1.0;
        OverworldShinyDetectedActionOption* shiny_action = nullptr;

        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            if (flag_distance.load(std::memory_order_acquire) <= ENROUTE_DISTANCE){
                shiny_action = &SHINY_DETECTED_DESTINATION;
            }else{
                shiny_action = &SHINY_DETECTED_ENROUTE;
            }
            return on_shiny_callback(env, env.console, *shiny_action, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                goto_camp_from_jubilife(env, env.console, context, TRAVEL_LOCATION, fresh_from_reset);
                FlagNavigationAir session(
                    env, env.console, context,
                    STOP_DISTANCE,
                    FLAG_REACHED_DELAY,
                    std::chrono::seconds(NAVIGATION_TIMEOUT)
                );
                session.set_distance_callback([&](double distance){
                    flag_distance.store(distance, std::memory_order_release);
                });
                session.run_session();
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0){
            on_shiny_sound(env, env.console, context, *shiny_action, shiny_coefficient);
        }

        if(RESET_METHOD == ResetMethod::SoftReset){
            env.console.log("Resetting by closing the game.");
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            fresh_from_reset = reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }else{
            env.console.log("Resetting by going to village.");
            goto_camp_from_overworld(env, env.console, context);
            goto_professor(env.console.logger(), context, TRAVEL_LOCATION);
            from_professor_return_to_jubilife(env, env.console, context);
        }
    }
}


void ShinyHuntFlagPin::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntFlagPin_Descriptor::Stats& stats = env.current_stats<ShinyHuntFlagPin_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context, fresh_from_reset);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            fresh_from_reset = reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
