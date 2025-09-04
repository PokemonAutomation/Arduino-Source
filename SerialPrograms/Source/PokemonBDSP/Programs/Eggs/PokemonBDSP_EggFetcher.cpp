/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggFetcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;



EggFetcher_Descriptor::EggFetcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:EggFetcher",
        STRING_POKEMON + " BDSP", "Egg Fetcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggFetcher.md",
        "Automatically fetch eggs from the daycare man.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct EggFetcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Fetch Attempts"])
    {
        m_display_order.emplace_back("Fetch Attempts");
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> EggFetcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggFetcher::EggFetcher()
    : GO_HOME_WHEN_DONE(false)
    , SHORTCUT("<b>Bike Shortcut:</b>")
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        LockMode::LOCK_WHILE_RUNNING,
        2000
    )
    , TRAVEL_TIME_PER_FETCH0(
        "<b>Travel Time per Fetch:</b><br>Fetch an egg after traveling for this long.",
        LockMode::LOCK_WHILE_RUNNING,
        "15 s"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(TRAVEL_TIME_PER_FETCH0);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void EggFetcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    EggFetcher_Descriptor::Stats& stats = env.current_stats<EggFetcher_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    //  Move to corner.
    pbf_move_left_joystick(context, 0, 255, 125, 0);

    for (uint16_t c = 0; c < MAX_FETCH_ATTEMPTS; c++){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        egg_spin_with_A(context, TRAVEL_TIME_PER_FETCH0);
        SHORTCUT.run(context, 100);

        //  Move to man.
        pbf_move_left_joystick(context, 0, 255, 30, 0);
        pbf_move_left_joystick(context, 128, 0, 35, 0);
        pbf_move_left_joystick(context, 255, 128, 60, 0);

        //  Fetch egg.
        pbf_mash_button(context, BUTTON_ZL, 600);
        pbf_mash_button(context, BUTTON_B, 520);
        pbf_move_left_joystick(context, 0, 255, 125, 0);
        SHORTCUT.run(context, 100);

        stats.m_attempts++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
