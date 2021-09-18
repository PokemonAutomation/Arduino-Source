/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_LobbyWait.h"
#include "PokemonSwSh_AutoHostStats.h"
#include "PokemonSwSh_AutoHost-Rolling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


AutoHostRolling_Descriptor::AutoHostRolling_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:AutoHostRolling",
        "Auto-Host Rolling",
        "SwSh-Arduino/wiki/Basic:-AutoHost-Rolling",
        "Roll N days, host, SR and repeat. Also supports hard-locks and soft-locks.",
        FeedbackType::OPTIONAL_,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



AutoHostRolling::AutoHostRolling(const AutoHostRolling_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SKIPS("<b>Day Skips:</b>", 3)
    , BACKUP_SAVE("<b>Load Backup Save:</b><br>For backup save soft-locking method.", false)
    , HOST_ONLINE("<b>Host Online:</b>", true)
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        "60 * TICKS_PER_SECOND"
    )
    , FRIEND_ACCEPT_USER_SLOT(
        "<b>Friend Request Accept Slot:</b><br>Zero disables friend accepts.",
        0, 0, 8
    )
    , EXTRA_DELAY_BETWEEN_RAIDS(
        "<b>Extra Delay Between Raids:</b><br>May aid in farming.",
        "0 * TICKS_PER_SECOND"
    )
    , MOVE_SLOT(
        "<b>1st Move Select Slot:</b><br>Zero disables 1st move select.",
        0, 0, 4
    )
    , DYNAMAX(
        "<b>1st Move Dynamax:</b><br>Dynamax on first move. (only applies if above option is non-zero)",
        true
    )
    , TROLL_HOSTING(
        "<b>Troll Hosting:</b> (requires 1st move select)<br>0 disables the troll hosting option, 1 attacks the first ally, 2 attacks the second one, 3 attacks the third one. Dynamaxing will disable this option.",
        0, 0, 3
    )
    , ALTERNATE_GAMES(
        "<b>Alternate Games:</b><br>Alternate hosting between 1st and 2nd games. Host from both Sword and Shield.",
        false
    )
    , NOTIFICATIONS("Discord Notifications")
    , m_internet_settings(
        "<font size=4><b>Internet Settings:</b> Increase these if your internet is slow.</font>"
    )
    , CONNECT_TO_INTERNET_DELAY(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to enter den.",
        "20 * TICKS_PER_SECOND"
    )
    , ENTER_ONLINE_DEN_DELAY(
        "<b>Enter Online Den Delay:</b><br>\"Communicating\" when entering den while online.",
        "8 * TICKS_PER_SECOND"
    )
    , OPEN_ONLINE_DEN_LOBBY_DELAY(
        "<b>Open Online Den Delay:</b><br>Delay from \"Invite Others\" to when the clock starts ticking.",
        "8 * TICKS_PER_SECOND"
    )
    , RAID_START_TO_EXIT_DELAY(
        "<b>Raid Start to Exit Delay:</b><br>Time from start raid to reset. (when not selecting move)",
        "15 * TICKS_PER_SECOND"
    )
    , DELAY_TO_SELECT_MOVE(
        "<b>Delay to Select Move:</b><br>This + above = time from start raid to select move.",
        "32 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(BACKUP_SAVE);
    PA_ADD_OPTION(HOST_ONLINE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(CATCHABILITY);
    PA_ADD_OPTION(FRIEND_ACCEPT_USER_SLOT);
    PA_ADD_OPTION(EXTRA_DELAY_BETWEEN_RAIDS);
    PA_ADD_OPTION(MOVE_SLOT);
    PA_ADD_OPTION(DYNAMAX);
    PA_ADD_OPTION(TROLL_HOSTING);
    PA_ADD_OPTION(ALTERNATE_GAMES);
    if (PERSISTENT_SETTINGS().developer_mode){
        PA_ADD_OPTION(NOTIFICATIONS);
    }

    PA_ADD_OPTION(m_internet_settings);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY);
    PA_ADD_OPTION(ENTER_ONLINE_DEN_DELAY);
    PA_ADD_OPTION(OPEN_ONLINE_DEN_LOBBY_DELAY);
    PA_ADD_OPTION(RAID_START_TO_EXIT_DELAY);
    PA_ADD_OPTION(DELAY_TO_SELECT_MOVE);
}



std::unique_ptr<StatsTracker> AutoHostRolling::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoHostStats());
}


void AutoHostRolling::program(SingleSwitchProgramEnvironment& env){
    AutoHostStats& stats = env.stats<AutoHostStats>();

    uint16_t start_raid_delay = HOST_ONLINE
        ? OPEN_ONLINE_DEN_LOBBY_DELAY
        : OPEN_LOCAL_DEN_LOBBY_DELAY;
    const uint16_t lobby_wait_delay = LOBBY_WAIT_DELAY < start_raid_delay
        ? 0
        : LOBBY_WAIT_DELAY - start_raid_delay;

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
    }

    uint32_t last_touch = 0;
    if (SKIPS == 0 && TOUCH_DATE_INTERVAL > 0){
        touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
        last_touch = system_clock(env.console);
    }
    rollback_date_from_home(env.console, SKIPS);
    resume_game_front_of_den_nowatts(env.console, TOLERATE_SYSTEM_UPDATE_MENU_SLOW);

    char first = true;
    for (uint32_t raids = 0;; raids++){
        env.log("Raids Completed: " + tostr_u_commas(raids));
        env.update_stats();

        roll_den(
            env.console,
            ENTER_ONLINE_DEN_DELAY,
            OPEN_ONLINE_DEN_LOBBY_DELAY,
            SKIPS,
            CATCHABILITY
        );

        if (HOST_ONLINE){
            connect_to_internet(env.console, OPEN_YCOMM_DELAY, CONNECT_TO_INTERNET_DELAY);
        }

        env.console.botbase().wait_for_all_requests();
        {
            DenMonReader reader(env.console, env.console);
            enter_den(env.console, ENTER_ONLINE_DEN_DELAY, SKIPS != 0, HOST_ONLINE);

            //  Don't delay if it's the first iteration.
            if (first){
                first = false;
            }else{
                pbf_wait(env.console, EXTRA_DELAY_BETWEEN_RAIDS);
            }

            uint8_t code[8];
            bool has_code = RAID_CODE.get_code(code);
            if (has_code){
                char str[8];
                for (size_t c = 0; c < 8; c++){
                    str[c] = code[c] + '0';
                }
                env.log("Next Raid Code: " + std::string(str, sizeof(str)));
                pbf_press_button(env.console, BUTTON_PLUS, 5, 145);
                enter_digits(env.console, 8, code);
                pbf_wait(env.console, 180);
                pbf_press_button(env.console, BUTTON_A, 5, 95);
            }
            env.console.botbase().wait_for_all_requests();

            send_raid_notification(
                descriptor().display_name(),
                env.console,
                NOTIFICATIONS,
                has_code, code,
                reader, stats
            );
        }

        enter_lobby(env.console, OPEN_ONLINE_DEN_LOBBY_DELAY, HOST_ONLINE, CATCHABILITY);

        //  Accept friend requests while we wait.
        RaidLobbyState raid_state = raid_lobby_wait(
            env.console,
            HOST_ONLINE,
            FRIEND_ACCEPT_USER_SLOT,
            lobby_wait_delay
        );

        //  Start Raid
        pbf_press_dpad(env.console, DPAD_UP, 5, 45);

        //  Mash A until it's time to close the game.
#if 1
        {
            env.console.botbase().wait_for_all_requests();
            uint32_t start = system_clock(env.console);
            pbf_mash_button(env.console, BUTTON_A, 3 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            BlackScreenDetector black_screen;
            uint32_t now = start;
            while (true){
                if (black_screen.black_is_over(env.console.video().snapshot())){
                    env.log("Raid has Started!", "blue");
                    stats.add_raid(raid_state.raiders());
                    break;
                }
                if (now - start >= RAID_START_TO_EXIT_DELAY){
                    stats.add_timeout();
                    break;
                }
                pbf_mash_button(env.console, BUTTON_A, TICKS_PER_SECOND);
                env.console.botbase().wait_for_all_requests();
                now = system_clock(env.console);
            }
        }
#else
        pbf_mash_button(BUTTON_A, RAID_START_TO_EXIT_DELAY);
#endif

        //  Select a move.
        if (MOVE_SLOT > 0){
            pbf_wait(env.console, DELAY_TO_SELECT_MOVE);
            pbf_press_button(env.console, BUTTON_A, 20, 80);
            if (DYNAMAX){
                pbf_press_dpad(env.console, DPAD_LEFT, 20, 30);
                pbf_press_button(env.console, BUTTON_A, 20, 60);
            }
            for (uint8_t c = 1; c < MOVE_SLOT; c++){
                pbf_press_dpad(env.console, DPAD_DOWN, 20, 30);
            }
            pbf_press_button(env.console, BUTTON_A, 20, 80);

            // Disable the troll hosting option if the dynamax is set to TRUE.
            if (!DYNAMAX && TROLL_HOSTING > 0){
                pbf_press_dpad(env.console, DPAD_DOWN, 20, 80);
                for (uint8_t c = 0; c < TROLL_HOSTING; c++){
                    pbf_press_dpad(env.console, DPAD_RIGHT, 20, 80);
                }
            }

            pbf_press_button(env.console, BUTTON_A, 20, 980);
        }

        //  Add a little extra wait time since correctness matters here.
        ssf_press_button2(env.console, BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);

        close_game(env.console);

        //  Touch the date.
        if (SKIPS == 0 && TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
            touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
            last_touch += TOUCH_DATE_INTERVAL;
        }
        rollback_date_from_home(env.console, SKIPS);

        start_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
            0, 0,
            BACKUP_SAVE
        );
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}







