/*  Walking Pokemon Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_AmitySquarePickUpFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


AmitySquarePickUpFarmer_Descriptor::AmitySquarePickUpFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:AmitySquarePickUpFarmer",
        STRING_POKEMON + " BDSP", "Amity Square Pick Up Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/AmitySquarePickUpFarmer.md",
        "Automatically fetch berries and stickers from the walking pokemon in Amity Square.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct AmitySquarePickUpFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Fetch Attempts"])
    {
        m_display_order.emplace_back("Fetch Attempts");
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> AmitySquarePickUpFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AmitySquarePickUpFarmer::AmitySquarePickUpFarmer()
    : GO_HOME_WHEN_DONE(false)
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many items you can get.",
        LockMode::LOCK_WHILE_RUNNING,
        100
    )
    , ONE_WAY_MOVING_TIME0(
        "<b>One Way walking Time:</b><br>Walk this amount of time in one direction before going back to finish one round of walking.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , ROUNDS_PER_FETCH(
        "<b>Rounds per fetch:</b><br>How many rounds of walking before doing a berry fetch attempt.",
        LockMode::LOCK_WHILE_RUNNING,
        3
    )
    , WAIT_TIME_FOR_POKEMON0(
        "<b>Wait Time for Pokemon:</b><br>Wait this time for pokemon to catch up to you before you ask for a berry.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(ONE_WAY_MOVING_TIME0);
    PA_ADD_OPTION(ROUNDS_PER_FETCH);
    PA_ADD_OPTION(WAIT_TIME_FOR_POKEMON0);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void AmitySquarePickUpFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AmitySquarePickUpFarmer_Descriptor::Stats& stats = env.current_stats<AmitySquarePickUpFarmer_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    for (uint16_t c = 0; c < MAX_FETCH_ATTEMPTS; c++){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        for (uint16_t i = 0; i < ROUNDS_PER_FETCH; i++){
            //  Move right
            pbf_move_left_joystick(context, 255, 128, ONE_WAY_MOVING_TIME0, 0ms);
            // Move left
            pbf_move_left_joystick(context, 0, 128, ONE_WAY_MOVING_TIME0, 0ms);
        }

        // Wait for your pokemon to catch up to you
        pbf_wait(context, WAIT_TIME_FOR_POKEMON0);

        // Face toward your pokemon.
        pbf_press_dpad(context, DPAD_RIGHT, 1, 0);

        // Mash button to talk to pokemon
        pbf_mash_button(context, BUTTON_ZL, 500);

        // Mash button to end talking to pokemon
        pbf_mash_button(context, BUTTON_B, 500);

        stats.m_attempts++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
