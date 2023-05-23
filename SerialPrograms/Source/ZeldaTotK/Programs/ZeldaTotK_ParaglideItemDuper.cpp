/*  TotK Paraglide Item Duper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "ZeldaTotK_ParaglideItemDuper.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace ZeldaTotK {

ParaglideItemDuper_Descriptor::ParaglideItemDuper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ZeldaTotK:ParaglideItemDuper",
        "Zelda: TotK", "Paraglide Item Duper",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/ParaglideItemDuper.md",
        "Use the Paraglide Menu Sort glitch to duplicate items.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct ParaglideItemDuper_Descriptor::Stats : public StatsTracker {
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Dupe Attempts");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& dupe_attempts;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ParaglideItemDuper_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}

ParaglideItemDuper::ParaglideItemDuper()
    : ATTEMPTS(
        "<b>Duplication Attempts:</b><br>The number of times you wish to run this routine.",
        LockWhileRunning::UNLOCKED,
        100
    )
    , LOAD_DELAY(
        "<b>Loading time:</b><br>Adjustable delay for the load screen after warping. "
        "The default of 3750 ticks is 30 seconds.",
        LockWhileRunning::UNLOCKED,
        TICKS_PER_SECOND,
        "3750"
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        // &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(ATTEMPTS);
    PA_ADD_OPTION(LOAD_DELAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ParaglideItemDuper::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    ParaglideItemDuper_Descriptor::Stats& stats = env.current_stats<ParaglideItemDuper_Descriptor::Stats>();

    /*
    Setup:
    Menu has been opened to materials tab and sorted by type
    Item you wish to dupe is selected in menu
    Now close the menu

    You are at the shrine near link's (new) house
    Map tab is selected and is on the second to last zoom level
    
    Put the sages away since they can interfere with pickup
    */

    uint32_t c = 0;

    while (c < ATTEMPTS) {
        env.log("Current Attempts: " + tostr_u_commas(c));

        //Walk forward from teleportation point
        pbf_move_left_joystick(context, 128, 0, 10, 5);

        //Jump and open paraglider
        pbf_press_button(context, BUTTON_X, 20, 20);
        pbf_press_button(context, BUTTON_X, 20, 20);

        //Open menu
        pbf_press_button(context, BUTTON_PLUS, 20, 100);

        //Select 5 of the item to dupe
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_dpad(context, DPAD_DOWN, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);

        //Now press SORT and B at the same time, or as close as possible
        ssf_press_button(context, BUTTON_B, 0, 10);
        ssf_press_button(context, BUTTON_Y, 0, 10);

        pbf_wait(context, 50);
        context.wait_for_all_requests();

        //Close paraglider and drop to the ground
        pbf_press_button(context, BUTTON_B, 20, 50);

        //Pick up duped items
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);

        //Open map and teleport back
        pbf_press_button(context, BUTTON_MINUS, 20, 10);
        pbf_move_left_joystick(context, 128, 255, 10, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);

        //Wait for loading screen
        pbf_wait(context, LOAD_DELAY);
        context.wait_for_all_requests();

        // increment counter, increment stats
        c++;
        stats.dupe_attempts++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

