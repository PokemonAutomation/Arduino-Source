/*  TotK Surf Item Duper
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
#include "ZeldaTotK_SurfItemDuper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

SurfItemDuper_Descriptor::SurfItemDuper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ZeldaTotK:SurfItemDuper",
        "Zelda: TotK", "Shield Surf Item Duper",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/SurfItemDuper.md",
        "Use the Shield Surfing Menu Sort glitch to duplicate items.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct SurfItemDuper_Descriptor::Stats : public StatsTracker{
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
    {
        m_display_order.emplace_back("Dupe Attempts");
    }
    std::atomic<uint64_t>& dupe_attempts;
};
std::unique_ptr<StatsTracker> SurfItemDuper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

SurfItemDuper::SurfItemDuper()
    : ATTEMPTS(
        "<b>Duplication Attempts:</b><br>The number of times you wish to run this routine.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
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
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SurfItemDuper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    SurfItemDuper_Descriptor::Stats& stats = env.current_stats<SurfItemDuper_Descriptor::Stats>();

    /*
    Setup:
    Stand on a patch of sand in a shrine or in the depths (under a lightroot)

    Menu has been opened to materials tab
    Item you wish to dupe is selected in menu
    Now close the menu

    Durability should not matter when on sand or snow
    
    Sages being out doesn't matter as they don't appear inside shrines
    Blood moons don't matter inside shrines
    Heat/Cold don't matter inside shrines

    Some items may bounce more, so all 5 dupes are not always picked up (ex large batteries may roll away)
    Works with less than 5 of an item but tends to be less consistent

    Don't try this with bombs or anything explosive

    Works on 1.1 and 1.1.1
    */

    uint32_t c = 0;

    while (c < ATTEMPTS){
        env.log("Current Attempts: " + tostr_u_commas(c));

        //Initiate Shield Surf
        ssf_press_button(context, BUTTON_ZL, 0ms, 640ms);
        ssf_press_button(context, BUTTON_X, 40ms, 240ms);
        ssf_press_button(context, BUTTON_A, 80ms, 640ms);

        //Open menu
        pbf_press_button(context, BUTTON_PLUS, 20, 100);

        //Select 5 of the item to dupe - if less than 5 this still works
        pbf_press_button(context, BUTTON_X, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);

        //Now press Y (sort) and B (exit) at the same time
        pbf_press_button(context, BUTTON_B | BUTTON_Y, 10, 40);

        //Drop to the ground
        pbf_press_button(context, BUTTON_B, 20, 50);

        //Pick up duped items - extra presses just in case
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);

        //Turn around and try to pick up items as well
        pbf_move_left_joystick(context, 128, 255, 10, 5);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);

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

