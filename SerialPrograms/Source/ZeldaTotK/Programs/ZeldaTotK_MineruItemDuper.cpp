/*  TotK Mineru Item Duper
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
#include "ZeldaTotK_MineruItemDuper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

MineruItemDuper_Descriptor::MineruItemDuper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ZeldaTotK:MineruItemDuper",
        "Zelda: TotK", "Mineru Item Duper",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/MineruItemDuper.md",
        "Use the Mineru Menu Sort glitch to duplicate items.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct MineruItemDuper_Descriptor::Stats : public StatsTracker{
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
    {
        m_display_order.emplace_back("Dupe Attempts");
    }
    std::atomic<uint64_t>& dupe_attempts;
};
std::unique_ptr<StatsTracker> MineruItemDuper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

MineruItemDuper::MineruItemDuper()
    : AMOUNT(
        "<b>Duplication Amount:</b><br>The number of items you wish to duplicate.",
        LockMode::LOCK_WHILE_RUNNING,
        900
    )
    , IS_ZONAI_DEVICE(
        "<b>Is Zonai Device:</b><br>Whether or not you are duplicating a Zonai device, or just a normal material.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        // &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(AMOUNT);
    PA_ADD_OPTION(IS_ZONAI_DEVICE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void MineruItemDuper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    MineruItemDuper_Descriptor::Stats& stats = env.current_stats<MineruItemDuper_Descriptor::Stats>();

    /*
    Setup:
    Menu has been opened to materials tab
    Item you wish to dupe is selected in menu
    Now close the menu

    Warp to The Depths to run this without having to worry about blood moons. All of my testing was done at
    the Great Abandoned Central Mine. 

    Mount Mineru. Ideally, avoid having other Sages near you to prevent interacting with them.

    If you are duping Zonai devices, you will need to be right next to a wall. Otherwise, try to stay in a
    flat, open area.

    ITEMS_PER_ATTEMPT will be 5 for normal materials and 10 for Zonai devices.

    Works on 1.1 and 1.1.1
    */

    uint32_t ITEMS_PER_ATTEMPT = IS_ZONAI_DEVICE ? 10 : 5;
    uint32_t c = 0;
    while (c < AMOUNT){
        env.log("Current Amount: " + tostr_u_commas(c));

        // Open menu
        pbf_press_button(context, BUTTON_PLUS, 20, 100);

        // Hold items to dupe
        pbf_press_button(context, BUTTON_A, 10, 10);

        // Pressing "Hold Item" is only necessary for Materials
        if (!IS_ZONAI_DEVICE){
            pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        }

        pbf_press_button(context, BUTTON_A, 10, 10);
         for (uint32_t i = 0; i < ITEMS_PER_ATTEMPT && c < AMOUNT; i++){
            pbf_press_button(context, BUTTON_A, 10, 10);

            // Increment counters
            c++;
            stats.dupe_attempts++;
        }

        // Now press Y (sort) and B (exit) at the same time
        ssf_press_button(context, BUTTON_B, 0ms, 80ms);
        pbf_press_button(context, BUTTON_Y, 80ms, 320ms);

        // There is no need to pick up Zonai devices 
        if (!IS_ZONAI_DEVICE){
            // Pick up duped items plus some extra attempts for potentially missed ones
            for (uint32_t i = 0; i < ITEMS_PER_ATTEMPT + 2; i++){
                pbf_press_button(context, BUTTON_A, 10, 10);
            }
        }else{
            pbf_wait(context, 250);
        }

        // increment stats
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

