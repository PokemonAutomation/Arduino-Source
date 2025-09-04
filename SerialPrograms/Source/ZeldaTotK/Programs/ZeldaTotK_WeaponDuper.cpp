/*  TotK Weapon Duper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "ZeldaTotK_WeaponDuper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

WeaponDuper_Descriptor::WeaponDuper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ZeldaTotK:WeaponDuper",
        "Zelda: TotK", "Weapon Duper",
        "ComputerControl/blob/master/Wiki/Programs/ZeldaTotK/WeaponDuper.md",
        "Use a glitch to duplicate your weapons (Bows, Shields and Swords)",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct WeaponDuper_Descriptor::Stats : public StatsTracker{
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
    {
        m_display_order.emplace_back("Dupe Attempts");
    }
    std::atomic<uint64_t>& dupe_attempts;
};
std::unique_ptr<StatsTracker> WeaponDuper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

WeaponDuper::WeaponDuper()
    : ATTEMPTS(
        "<b>Duplication Attempts:</b><br>The number of times you wish to run this routine.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10
    )
    , WEAPON_MENU_BUTTON_COUNT(
        "<b>Menu change button presses: :</b><br>How many button presses are required to go from option menu to weapon menu (bow = 7, shield = 6, sword = 5).",
        LockMode::UNLOCK_WHILE_RUNNING,
        5
    )
    , DPAD_RIGHT_BUTTON_COUNT(
        "<b>Right button presses: :</b><br>How many dpad right button presses are required to go from the left-most slot to the one you want to dupe.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , DPAD_DOWN_BUTTON_COUNT(
        "<b>Down button presses: :</b><br>How many dpad down button presses are required to go from the top-most slot to the one you want to dupe.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
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
    PA_ADD_OPTION(WEAPON_MENU_BUTTON_COUNT);
    PA_ADD_OPTION(DPAD_RIGHT_BUTTON_COUNT);
    PA_ADD_OPTION(DPAD_DOWN_BUTTON_COUNT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void WeaponDuper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    WeaponDuper_Descriptor::Stats& stats = env.current_stats<WeaponDuper_Descriptor::Stats>();

    uint32_t c = 0;

    while (c < ATTEMPTS){
        env.log("Current Attempts: " + tostr_u_commas(c));

        env.log("Open menu");
        pbf_press_button(context, BUTTON_PLUS, 10, 30);

        context.wait_for_all_requests();
        env.log("Go to options menu");
        for (uint32_t i = 0; i < WEAPON_MENU_BUTTON_COUNT; ++i)
        {
            pbf_press_button(context, BUTTON_R, 10, 30);
        }

        context.wait_for_all_requests();
        env.log("Save game");
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_dpad(context, DPAD_UP, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_wait(context, 5 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.log("Go to weapon menu");
        for (uint32_t i = 0; i < WEAPON_MENU_BUTTON_COUNT; ++i)
        {
            pbf_press_button(context, BUTTON_L, 10, 30);
        }

        context.wait_for_all_requests();
        env.log("Come back to first slot");
        for (uint32_t i = 0; i < 5; ++i)
        {
            pbf_press_dpad(context, DPAD_UP, 10, 30);
            pbf_press_dpad(context, DPAD_LEFT, 10, 30);
        }

        context.wait_for_all_requests();
        env.log("Go to desired slot");
        for (uint32_t i = 0; i < DPAD_RIGHT_BUTTON_COUNT; ++i)
        {
            pbf_press_dpad(context, DPAD_RIGHT, 10, 30);
        }
        for (uint32_t i = 0; i < DPAD_DOWN_BUTTON_COUNT; ++i)
        {
            pbf_press_dpad(context, DPAD_DOWN, 10, 30);
        }

        context.wait_for_all_requests();
        env.log("Drop weapon");
        pbf_press_button(context, BUTTON_A, 10, 30);
        pbf_press_dpad(context, DPAD_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 50);

        context.wait_for_all_requests();
        env.log("Equip weapon on the right");
        pbf_press_dpad(context, DPAD_RIGHT, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 30);

        context.wait_for_all_requests();
        env.log("Open and close the menu fast");
        pbf_press_button(context, BUTTON_PLUS, 10, 10);
        pbf_press_button(context, BUTTON_PLUS, 10, 100);

        context.wait_for_all_requests();
        env.log("Drop weapon");
        pbf_press_dpad(context, DPAD_LEFT, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 30);
        pbf_press_dpad(context, DPAD_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 50);

        context.wait_for_all_requests();
        env.log("Go back to options menu");
        for (uint32_t i = 0; i < WEAPON_MENU_BUTTON_COUNT; ++i)
        {
            pbf_press_button(context, BUTTON_R, 10, 30);
        }

        context.wait_for_all_requests();
        env.log("Load the save");
        pbf_press_dpad(context, DPAD_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_dpad(context, DPAD_UP, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_wait(context, 25 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.log("Pick up duped weapon - extra presses just in case");
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);

        context.wait_for_all_requests();
        env.log("Turn around and try to pick up items as well");
        pbf_move_left_joystick(context, 128, 255, 10, 5);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_move_left_joystick(context, 128, 0, 10, 5);

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

