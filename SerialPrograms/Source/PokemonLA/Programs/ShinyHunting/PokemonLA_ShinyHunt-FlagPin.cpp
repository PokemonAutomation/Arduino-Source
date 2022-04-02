/*  Shiny Hunt - Fixed Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_FlagNavigationAir.h"
#include "PokemonLA_ShinyHunt-FlagPin.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinyHuntFlagPin_Descriptor::ShinyHuntFlagPin_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-FlagPin",
        STRING_POKEMON + " LA", "Shiny Hunt - Flag Pin",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-FlagPin.md",
        "Repeatedly travel to a flag pin to shiny hunt " + STRING_POKEMON + " around it.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


ShinyHuntFlagPin::ShinyHuntFlagPin(const ShinyHuntFlagPin_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , STOP_DISTANCE(
        "<b>Stop Distance:</b><br>"
        "You have reached the flag when you come within this distance of it. "
        "Don't set this too small. The navigation is not precise enough to land directly on the flag.",
        20
    )
    , FLAG_REACHED_DELAY(
        "<b>Target Reached Delay:</b><br>"
        "Once you have reached the flag, wait this many seconds to ensure everything loads and that any shinies are heard before resetting.",
        1.0, 0, 60
    )
    , NAVIGATION_TIMEOUT(
        "<b>Navigation Timeout:</b><br>Give up and reset if flag is not reached after this many seconds.",
        180, 0
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(STOP_DISTANCE);
    PA_ADD_OPTION(FLAG_REACHED_DELAY);
    PA_ADD_OPTION(NAVIGATION_TIMEOUT);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

class ShinyHuntFlagPin::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies", true);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};

std::unique_ptr<StatsTracker> ShinyHuntFlagPin::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void ShinyHuntFlagPin::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    goto_camp_from_jubilife(env, context, env.console, TRAVEL_LOCATION);

    {
        FlagNavigationAir session(
            env, env.console,
            SHINY_DETECTED.stop_on_shiny(),
            STOP_DISTANCE,
            FLAG_REACHED_DELAY,
            std::chrono::seconds(NAVIGATION_TIMEOUT)
        );
        session.run_session();

        if (session.detected_shiny()){
            stats.shinies++;
            on_shiny_sound(env, context, env.console, SHINY_DETECTED, session.shiny_sound_results());
        }
    }

    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, context, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void ShinyHuntFlagPin::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env, context);
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, context, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}





}
}
}
