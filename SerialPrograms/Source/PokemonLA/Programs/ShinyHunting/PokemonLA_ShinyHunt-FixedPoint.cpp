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
#include "PokemonLA_ShinyHunt-FixedPoint.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


WarpLocationOption::WarpLocationOption()
    : EnumDropdownOption(
        "<b>Warp Spot</b><br>Travel from this location.",
        std::vector<QString>(WARP_SPOT_NAMES + 0, WARP_SPOT_NAMES + (size_t)WarpSpot::END_LIST),
        0
    )
{}




ShinyHuntFixedPoint_Descriptor::ShinyHuntFixedPoint_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-FixedPoint",
        STRING_POKEMON + " LA", "Shiny Hunt - Fixed Point",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-FixedPoint.md",
        "Repeatedly travel to a flag pin to shiny hunt " + STRING_POKEMON + " around it.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


ShinyHuntFixedPoint::ShinyHuntFixedPoint(const ShinyHuntFixedPoint_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NAVIGATION_TIMEOUT(
        "<b>Navigation Timeout:</b><br>Give up and reset if flag is not reached after this many seconds.",
        180, 0
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(WARP_SPOT);
    PA_ADD_OPTION(NAVIGATION_TIMEOUT);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

class ShinyHuntFixedPoint::Stats : public StatsTracker, public ShinyStatIncrementer{
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

std::unique_ptr<StatsTracker> ShinyHuntFixedPoint::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void ShinyHuntFixedPoint::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    goto_camp_from_jubilife(env, env.console, WARP_SPOT);

    {
        FlagNavigationAir session(
            env, env.console,
            SHINY_DETECTED.stop_on_shiny(),
            std::chrono::seconds(NAVIGATION_TIMEOUT)
        );
        session.run_session();

        if (session.detected_shiny()){
            stats.shinies++;
            on_shiny_sound(env, env.console, SHINY_DETECTED, session.shiny_sound_results());
        }
    }

    pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void ShinyHuntFixedPoint::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env);
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }catch (OperationCancelledException&){
            break;
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
