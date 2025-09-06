/*  OHKO Raid Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h"
#include "PokemonSwSh_RaidItemFarmerOKHO.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


RaidItemFarmerOHKO_Descriptor::RaidItemFarmerOHKO_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:RaidItemFarmerOHKO",
        STRING_POKEMON + " SwSh", "Raid Item Farmer (OHKO)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/RaidItemFarmerOHKO.md",
        "Farm items from raids that can be OHKO'ed. (requires multiple Switches)",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        2, 4, 2
    )
{}



RaidItemFarmerOHKO::RaidItemFarmerOHKO()
    : BACKUP_SAVE(
        "<b>Load Backup Save:</b><br>For backup save soft-locking method.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
//    , m_advanced_options(
//        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
//    )
    , WAIT_FOR_STAMP_DELAY0(
        "<b>Wait for Stamp Delay:</b><br>Wait this long for the stamp to show up.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , ENTER_STAMP_MASH_DURATION0(
        "<b>Enter Stamp Mash Duration:</b><br>Mash A this long to enter a raid from its stamp.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , RAID_START_MASH_DURATION0(
        "<b>Raid Start Mash Duration:</b><br>Mash A this long to start raid.",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
    , RAID_START_TO_ATTACK_DELAY0(
        "<b>Raid Start to Attack Delay:</b><br>Time from start raid to when the raiders attack.<br>"
        "Do not over-optimize this timing unless you are running with 4 Switches. The Wishiwashi NPC will break the program.",
        LockMode::LOCK_WHILE_RUNNING,
        "30 s"
    )
    , ATTACK_TO_CATCH_DELAY0(
        "<b>Attack to Catch Delay:</b><br>Time from when you attack to when the catch selection appears.<br>"
        "Do not over-optimize this timing unless you are running with 4 Switches. The Clefairy NPC's Follow Me will break the program.",
        LockMode::LOCK_WHILE_RUNNING,
        "18 s"
    )
    , RETURN_TO_OVERWORLD_DELAY0(
        "<b>Return to Overworld Delay:</b><br>Time from when you don't catch to when you return to the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "18 s"
    )
    , TOUCH_DATE_INTERVAL0(
        "<b>Rollover Prevention:</b><br>Prevent the den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        LockMode::LOCK_WHILE_RUNNING,
        "4 hours"
    )
{
    PA_ADD_OPTION(BACKUP_SAVE);
//    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(WAIT_FOR_STAMP_DELAY0);
    PA_ADD_OPTION(ENTER_STAMP_MASH_DURATION0);
    PA_ADD_OPTION(RAID_START_MASH_DURATION0);
    PA_ADD_OPTION(RAID_START_TO_ATTACK_DELAY0);
    PA_ADD_OPTION(ATTACK_TO_CATCH_DELAY0);
    PA_ADD_OPTION(RETURN_TO_OVERWORLD_DELAY0);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL0);
}

void RaidItemFarmerOHKO::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    ProControllerContext host(scope, env.consoles[0].controller<ProController>());
    size_t switches = env.consoles.size();

    WallDuration TOUCH_DATE_INTERVAL = TOUCH_DATE_INTERVAL0;

    env.run_in_parallel(
        scope,
        [](ConsoleHandle& console, ProControllerContext& context){
            StartProgramChecks::check_performance_class_wired_or_wireless(context);
            grip_menu_connect_go_home(context);
        }
    );

    WallClock last_touch = current_time();
    if (TOUCH_DATE_INTERVAL > 0ms){
        touch_date_from_home(env.consoles[0], host, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        last_touch = current_time();
    }
    env.run_in_parallel(
        scope,
        [](ConsoleHandle& console, ProControllerContext& context){
            if (console.index() == 0){
                resume_game_front_of_den_nowatts(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
            }else{
                resume_game_no_interact(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
            }
        }
    );


    for (uint32_t raids = 0;; raids++){
        env.log("Raids Completed: " + tostr_u_commas(raids));

        host.wait_for_all_requests();
        env.run_in_parallel(
            scope,
            [&](ConsoleHandle& console, ProControllerContext& context){
                if (console.index() == 0){
                    enter_den(context, 0ms, false, false);
                }else{
                    pbf_press_button(context, BUTTON_Y, 80ms, GameSettings::instance().OPEN_YCOMM_DELAY0);
                    pbf_press_dpad(context, DPAD_UP, 5, 0);
                    pbf_move_right_joystick(context, 128, 0, 5, 0);
                    pbf_press_button(context, BUTTON_X, 10, 10);
                }
            }
        );

        enter_lobby(host, 0ms, false, Catchability::ALWAYS_CATCHABLE);

        host.wait_for_all_requests();
        env.run_in_parallel(
            scope, 1, switches,
            [&](ConsoleHandle& console, ProControllerContext& context){
                pbf_wait(context, WAIT_FOR_STAMP_DELAY0);
                pbf_press_button(context, BUTTON_X, 10, 10);
                pbf_press_dpad(context, DPAD_RIGHT, 10, 10);
                pbf_mash_button(context, BUTTON_A, ENTER_STAMP_MASH_DURATION0);
            }
        );

        //  Start Raid
        pbf_press_dpad(host, DPAD_UP, 5, 45);

        host.wait_for_all_requests();
        env.run_in_parallel(
            scope,
            [&](ConsoleHandle& console, ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, RAID_START_MASH_DURATION0);
                pbf_wait(context, RAID_START_TO_ATTACK_DELAY0);
                pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
                pbf_wait(context, ATTACK_TO_CATCH_DELAY0);

                if (console.index() == 0){
                    //  Add a little extra wait time since correctness matters here.
                    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);

                    close_game_from_home(console, context);

                    //  Touch the date.
                    if (TOUCH_DATE_INTERVAL > 0ms && current_time() - last_touch >= TOUCH_DATE_INTERVAL){
                        touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
                        last_touch += TOUCH_DATE_INTERVAL;
                    }
                    start_game_from_home_with_inference(
                        console, context,
                        ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
                        0, 0,
                        BACKUP_SAVE
                    );
                }else{
                    pbf_press_dpad(context, DPAD_DOWN, 10, 10);
                    Milliseconds delay = RETURN_TO_OVERWORLD_DELAY0;
                    if (delay > 5000ms){
                        pbf_mash_button(context, BUTTON_A, delay - 5000ms);
                        pbf_mash_button(context, BUTTON_B, 5000ms);
                    }else{
                        pbf_mash_button(context, BUTTON_A, delay);
                    }
                }
            }
        );


    }

}



}
}
}
