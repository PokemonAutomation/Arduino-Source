/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_LobbyWait.h"
#include "PokemonSwSh_AutoHost-Rolling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

AutoHostRolling::AutoHostRolling()
    : SingleSwitchProgram(
        FeedbackType::OPTIONAL_, PABotBaseLevel::PABOTBASE_12KB,
        "Auto-Host Rolling",
        "NativePrograms/AutoHost-Rolling.md",
        "Roll N days, host, SR and repeat. Also supports hard-locks and soft-locks."
    )
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
    , ALTERNATE_GAMES(
        "<b>Alternate Games:</b><br>Alternate hosting between 1st and 2nd games. Host from both Sword and Shield.",
        false
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent the den from rolling over by periodically touching the date. If set to zero, this feature is disabled. If skips is non-zero, this feature is also disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
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
    m_options.emplace_back(&RAID_CODE, "RAID_CODE");
    m_options.emplace_back(&SKIPS, "SKIPS");
    m_options.emplace_back(&BACKUP_SAVE, "BACKUP_SAVE");
    m_options.emplace_back(&HOST_ONLINE, "HOST_ONLINE");
    m_options.emplace_back(&LOBBY_WAIT_DELAY, "LOBBY_WAIT_DELAY");
    m_options.emplace_back(&CATCHABILITY, "CATCHABILITY");
    m_options.emplace_back(&FRIEND_ACCEPT_USER_SLOT, "FRIEND_ACCEPT_USER_SLOT");
    m_options.emplace_back(&EXTRA_DELAY_BETWEEN_RAIDS, "EXTRA_DELAY_BETWEEN_RAIDS");
    m_options.emplace_back(&MOVE_SLOT, "MOVE_SLOT");
    m_options.emplace_back(&DYNAMAX, "DYNAMAX");
    m_options.emplace_back(&ALTERNATE_GAMES, "ALTERNATE_GAMES");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_internet_settings, "");
    m_options.emplace_back(&CONNECT_TO_INTERNET_DELAY, "CONNECT_TO_INTERNET_DELAY");
    m_options.emplace_back(&ENTER_ONLINE_DEN_DELAY, "ENTER_ONLINE_DEN_DELAY");
    m_options.emplace_back(&OPEN_ONLINE_DEN_LOBBY_DELAY, "OPEN_ONLINE_DEN_LOBBY_DELAY");
    m_options.emplace_back(&RAID_START_TO_EXIT_DELAY, "RAID_START_TO_EXIT_DELAY");
    m_options.emplace_back(&DELAY_TO_SELECT_MOVE, "DELAY_TO_SELECT_MOVE");
}

void AutoHostRolling::program(SingleSwitchProgramEnvironment& env) const{
    uint16_t start_raid_delay = HOST_ONLINE
        ? OPEN_ONLINE_DEN_LOBBY_DELAY
        : OPEN_LOCAL_DEN_LOBBY_DELAY;
    const uint16_t lobby_wait_delay = LOBBY_WAIT_DELAY < start_raid_delay
        ? 0
        : LOBBY_WAIT_DELAY - start_raid_delay;

    grip_menu_connect_go_home();

    uint32_t last_touch = 0;
    if (SKIPS == 0 && TOUCH_DATE_INTERVAL > 0){
        touch_date_from_home(SETTINGS_TO_HOME_DELAY);
        last_touch = system_clock();
    }
    rollback_date_from_home(SKIPS);
    resume_game_front_of_den_nowatts(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);

    char first = true;
    for (uint32_t raids = 0;; raids++){
        env.logger.log("Raids Completed: " + tostr_u_commas(raids));

        roll_den(ENTER_ONLINE_DEN_DELAY, OPEN_ONLINE_DEN_LOBBY_DELAY, SKIPS, CATCHABILITY);

        if (HOST_ONLINE){
            connect_to_internet(OPEN_YCOMM_DELAY, CONNECT_TO_INTERNET_DELAY);
        }
        enter_den(ENTER_ONLINE_DEN_DELAY, SKIPS != 0, HOST_ONLINE);

        //  Don't delay if it's the first iteration.
        if (first){
            first = false;
        }else{
            pbf_wait(EXTRA_DELAY_BETWEEN_RAIDS);
        }

        uint8_t code[8];
        if (RAID_CODE.get_code(code)){
            char str[8];
            for (size_t c = 0; c < 8; c++){
                str[c] = code[c] + '0';
            }
            env.logger.log("Next Raid Code: " + std::string(str, sizeof(str)));
            pbf_press_button(BUTTON_PLUS, 5, 145);
            enter_digits(8, code);
            pbf_wait(180);
            pbf_press_button(BUTTON_A, 5, 95);
        }
        enter_lobby(OPEN_ONLINE_DEN_LOBBY_DELAY, HOST_ONLINE, CATCHABILITY);

        //  Accept friend requests while we wait.
        raid_lobby_wait(
            env.console, env.logger,
            HOST_ONLINE,
            FRIEND_ACCEPT_USER_SLOT,
            lobby_wait_delay
        );

        //  Start Raid
        pbf_press_dpad(DPAD_UP, 5, 45);

        //  Mash A until it's time to close the game.
#if 1
        {
            env.console.botbase().wait_for_all_requests();
            uint32_t start = system_clock();
            pbf_mash_button(BUTTON_A, 3 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            BlackScreenDetector black_screen(env.console, env.logger);
            uint32_t now = start;
            while (now - start < RAID_START_TO_EXIT_DELAY){
                if (black_screen.black_is_over()){
                    env.logger.log("Raid has Started!", "blue");
                    break;
                }
                pbf_mash_button(BUTTON_A, TICKS_PER_SECOND);
                env.console.botbase().wait_for_all_requests();
                now = system_clock();
            }
        }
#else
        pbf_mash_button(BUTTON_A, RAID_START_TO_EXIT_DELAY);
#endif

        //  Select a move.
        if (MOVE_SLOT > 0){
            pbf_wait(DELAY_TO_SELECT_MOVE);
            pbf_press_button(BUTTON_A, 20, 80);
            if (DYNAMAX){
                pbf_press_dpad(DPAD_LEFT, 20, 30);
                pbf_press_button(BUTTON_A, 20, 60);
            }
            for (uint8_t c = 1; c < MOVE_SLOT; c++){
                pbf_press_dpad(DPAD_DOWN, 20, 30);
            }
            pbf_press_button(BUTTON_A, 20, 80);
            pbf_press_button(BUTTON_A, 20, 980);
        }

        //  Add a little extra wait time since correctness matters here.
        ssf_press_button2(BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);

        close_game();

        //  Touch the date.
        if (SKIPS == 0 && TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
            touch_date_from_home(SETTINGS_TO_HOME_DELAY);
            last_touch += TOUCH_DATE_INTERVAL;
        }
        rollback_date_from_home(SKIPS);
        start_game_from_home(
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
            0, 0,
            BACKUP_SAVE
        );
    }

    end_program_callback();
    end_program_loop();
}



}
}
}







