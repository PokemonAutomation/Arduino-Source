/*  TotK Paraglide Item Duper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "ZeldaTotK_ParaglideItemDuper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

ParaglideItemDuper_Descriptor::ParaglideItemDuper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ZeldaTotK:ParaglideItemDuper",
        "Zelda: TotK", "Paraglide Item Duper",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/ParaglideItemDuper.md",
        "Use the Paraglide Menu Sort glitch to duplicate items.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct ParaglideItemDuper_Descriptor::Stats : public StatsTracker{
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
    {
        m_display_order.emplace_back("Dupe Attempts");
    }
    std::atomic<uint64_t>& dupe_attempts;
};
std::unique_ptr<StatsTracker> ParaglideItemDuper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ParaglideItemDuper::ParaglideItemDuper()
    : ATTEMPTS(
        "<b>Duplication Attempts:</b><br>The number of times you wish to run this routine.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
    )
    , LOAD_DELAY0(
        "<b>Loading time:</b><br>Adjustable delay for the load screen after warping. "
        "Adjust this to match the loading time of your game.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "25 s"
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
    PA_ADD_OPTION(LOAD_DELAY0);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ParaglideItemDuper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    ParaglideItemDuper_Descriptor::Stats& stats = env.current_stats<ParaglideItemDuper_Descriptor::Stats>();

    /*
    Setup:
    Menu has been opened to materials tab
    Item you wish to dupe is selected in menu
    Now close the menu

    Warp point placed directly opposite the ladder (face slightly to the left of the ladder) in Lookout Landing
    angle away from the guard if possible to prevent talking to them.
    can always readjust if needed - sometimes the map places a stamp instead of warping if position is off
    
    Map tab is selected and is on the second to last zoom level (zoom all the way in and the go back one)
    
    Sages being out doesn't matter as they don't appear inside town

    Some items may bounce more, so all 5 dupes are not always picked up (ex large batteries may roll away)
    Works with less than 5 of an item but tends to be less consistent

    Don't try this with bombs or anything explosive
    Don't have any other stamps on the map in the area
    Unequip anything metal in case of thunderstorm
    again, do not try this with bombs

    Travel Medallion required.

    In theory this program can be modified to work with a shrine near a cliff. Needs to be in a safe location and sages need to be disabled.

    Works on 1.1 and 1.1.1
    */

    uint32_t c = 0;

    while (c < ATTEMPTS){
        env.log("Current Attempts: " + tostr_u_commas(c));

        //Walk forward from teleportation point
        pbf_move_left_joystick(context, 128, 0, 10, 5);

        //Jump and open paraglider
        pbf_press_button(context, BUTTON_X, 20, 80);
        pbf_press_button(context, BUTTON_X, 20, 20);

        //Open menu
        pbf_press_button(context, BUTTON_PLUS, 20, 100);

        //Select 5 of the item to dupe - if less than 5 this still works
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_dpad(context, DPAD_DOWN, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);
        pbf_press_button(context, BUTTON_A, 20, 20);

        //Now press Y (sort) and B (exit) at the same time
        ssf_press_button(context, BUTTON_B, 0ms, 80ms);
        pbf_press_button(context, BUTTON_Y, 80ms, 320ms);

        //Close paraglider and drop to the ground
        pbf_press_button(context, BUTTON_B, 20, 50);

        //Pick up duped items - extra presses just in case
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_A, 20, 10);

        //Turn around and try to pick up items as well
        pbf_move_left_joystick(context, 128, 255, 10, 5);
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
        pbf_wait(context, LOAD_DELAY0);
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

