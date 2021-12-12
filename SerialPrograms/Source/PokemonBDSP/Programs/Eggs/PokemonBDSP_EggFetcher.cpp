/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggFetcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


EggFetcher_Descriptor::EggFetcher_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:EggFetcher",
        STRING_POKEMON + " BDSP", "Egg Fetcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggFetcher.md",
        "Automatically fetch eggs from the daycare man.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


EggFetcher::EggFetcher(const EggFetcher_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , SHORTCUT("<b>Bike Shortcut:</b>")
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        2000
    )
    , TRAVEL_TIME_PER_FETCH(
        "<b>Travel Time per Fetch:</b><br>Fetch an egg after traveling for this long.",
        "15 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(TRAVEL_TIME_PER_FETCH);
    PA_ADD_OPTION(NOTIFICATIONS);
}


struct EggFetcher::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Fetch Attempts"])
    {
        m_display_order.emplace_back("Fetch Attempts");
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> EggFetcher::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void EggFetcher::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(env.console, 0, 255, 10, 0);

    //  Move to corner.
    pbf_move_left_joystick(env.console, 0, 255, 125, 0);

    for (uint16_t c = 0; c < MAX_FETCH_ATTEMPTS; c++){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        egg_spin(env.console, TRAVEL_TIME_PER_FETCH);
        SHORTCUT.run(env.console, 100);

        //  Move to man.
        pbf_move_left_joystick(env.console, 0, 255, 30, 0);
        pbf_move_left_joystick(env.console, 128, 0, 35, 0);
        pbf_move_left_joystick(env.console, 255, 128, 60, 0);

        //  Fetch egg.
        pbf_mash_button(env.console, BUTTON_ZL, 500);
        pbf_mash_button(env.console, BUTTON_B, 500);
        pbf_move_left_joystick(env.console, 0, 255, 125, 0);
        SHORTCUT.run(env.console, 100);

        stats.m_attempts++;
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}





}
}
}
