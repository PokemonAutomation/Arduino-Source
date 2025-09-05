/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_AutoHostStats.h"
#include "PokemonSwSh_AutoHost.h"
#include "PokemonSwSh_AutoHost-Rolling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


AutoHostRolling_Descriptor::AutoHostRolling_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:AutoHostRolling",
        STRING_POKEMON + " SwSh", "Auto-Host Rolling",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/AutoHost-Rolling.md",
        "Roll N days, host, SR and repeat. Also supports hard-locks and soft-locks.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> AutoHostRolling_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoHostStats());
}



AutoHostRolling::AutoHostRolling()
    : SingleSwitchProgramInstance({"Notifs", "LiveHost"})
    , RAID_CODE(8)
    , SKIPS("<b>Day Skips:</b>", LockMode::LOCK_WHILE_RUNNING, 3)
    , BACKUP_SAVE(
        "<b>Load Backup Save:</b><br>For backup save soft-locking method.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , HOST_ONLINE("<b>Host Online:</b>", LockMode::LOCK_WHILE_RUNNING, true)
    , LOBBY_WAIT_DELAY0(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
    , FRIEND_ACCEPT_USER_SLOT(
        "<b>Friend Request Accept Slot:</b><br>Zero disables friend accepts.",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 8
    )
    , EXTRA_DELAY_BETWEEN_RAIDS0(
        "<b>Extra Delay Between Raids:</b><br>May aid in farming.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 s"
    )
    , MOVE_SLOT(
        "<b>1st Move Select Slot:</b><br>Zero disables 1st move select.",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 4
    )
    , DYNAMAX(
        "<b>1st Move Dynamax:</b><br>Dynamax on first move. (only applies if above option is non-zero)",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TROLL_HOSTING(
        "<b>Troll Hosting:</b> (requires 1st move select)<br>0 disables the troll hosting option, 1 attacks the first ally, 2 attacks the second one, 3 attacks the third one. Dynamaxing will disable this option.",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 3
    )
    , ALTERNATE_GAMES(
        "<b>Alternate Games:</b><br>Alternate hosting between 1st and 2nd games. Host from both Sword and Shield.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , HOSTING_NOTIFICATIONS("Live-Hosting Announcements", false)
    , NOTIFICATIONS({
        &HOSTING_NOTIFICATIONS.NOTIFICATION,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_internet_settings(
        "<font size=4><b>Internet Settings:</b> Increase these if your internet is slow.</font>"
    )
    , CONNECT_TO_INTERNET_DELAY0(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to enter den.",
        LockMode::LOCK_WHILE_RUNNING,
        "60000 ms"
    )
    , ENTER_ONLINE_DEN_DELAY0(
        "<b>Enter Online Den Delay:</b><br>\"Communicating\" when entering den while online.",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
    , OPEN_ONLINE_DEN_LOBBY_DELAY0(
        "<b>Open Online Den Delay:</b><br>Delay from \"Invite Others\" to when the clock starts ticking.",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
    , RAID_START_TO_EXIT_DELAY0(
        "<b>Raid Start to Exit Delay:</b><br>Time from start raid to reset. (when not selecting move)",
        LockMode::LOCK_WHILE_RUNNING,
        "15 s"
    )
    , DELAY_TO_SELECT_MOVE0(
        "<b>Delay to Select Move:</b><br>This + above = time from start raid to select move.",
        LockMode::LOCK_WHILE_RUNNING,
        "32 s"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(BACKUP_SAVE);
    PA_ADD_OPTION(HOST_ONLINE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY0);
    PA_ADD_OPTION(CATCHABILITY);
    PA_ADD_OPTION(FRIEND_ACCEPT_USER_SLOT);
    PA_ADD_OPTION(EXTRA_DELAY_BETWEEN_RAIDS0);
    PA_ADD_OPTION(MOVE_SLOT);
    PA_ADD_OPTION(DYNAMAX);
    PA_ADD_OPTION(TROLL_HOSTING);
    PA_ADD_OPTION(ALTERNATE_GAMES);
    PA_ADD_OPTION(HOSTING_NOTIFICATIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_OPTION(m_internet_settings);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY0);
    PA_ADD_OPTION(ENTER_ONLINE_DEN_DELAY0);
    PA_ADD_OPTION(OPEN_ONLINE_DEN_LOBBY_DELAY0);
    PA_ADD_OPTION(RAID_START_TO_EXIT_DELAY0);
    PA_ADD_OPTION(DELAY_TO_SELECT_MOVE0);
}



void AutoHostRolling::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    Milliseconds start_raid_delay = HOST_ONLINE
        ? OPEN_ONLINE_DEN_LOBBY_DELAY0
        : GameSettings::instance().OPEN_LOCAL_DEN_LOBBY_DELAY0;
    const Milliseconds lobby_wait_delay = LOBBY_WAIT_DELAY0.get() < start_raid_delay
        ? 0ms
        : LOBBY_WAIT_DELAY0.get() - start_raid_delay;

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    if (SKIPS == 0){
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(env.console, context);
    }
    rollback_date_from_home(env.console, context, SKIPS);
    if (env.console.video().snapshot()){
        NintendoSwitch::resume_game_from_home(env.console, context);
    }else{
        resume_game_front_of_den_nowatts(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }

    for (uint32_t raids = 0;; raids++){
        env.log("Raids Attempted: " + tostr_u_commas(raids));
        env.update_stats();

        run_autohost(
            env, env.console, context,
            CATCHABILITY, SKIPS,
            &RAID_CODE, lobby_wait_delay,
            HOST_ONLINE, FRIEND_ACCEPT_USER_SLOT,
            MOVE_SLOT, DYNAMAX, TROLL_HOSTING,
            HOSTING_NOTIFICATIONS,
            CONNECT_TO_INTERNET_DELAY0,
            ENTER_ONLINE_DEN_DELAY0,
            OPEN_ONLINE_DEN_LOBBY_DELAY0,
            RAID_START_TO_EXIT_DELAY0,
            DELAY_TO_SELECT_MOVE0
        );

        //  Exit game.
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
        close_game_from_home(env.console, context);

        //  Post-raid delay.
        pbf_wait(context, EXTRA_DELAY_BETWEEN_RAIDS0);

        //  Touch the date.
        if (SKIPS == 0){
            TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(env.console, context);
        }else{
            rollback_date_from_home(env.console, context, SKIPS);
        }

        start_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
            ALTERNATE_GAMES ? 2 : 0, 0,
            BACKUP_SAVE
        );
    }
}



}
}
}







