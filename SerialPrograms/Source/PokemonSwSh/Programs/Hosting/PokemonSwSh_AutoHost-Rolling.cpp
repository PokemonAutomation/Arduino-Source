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
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
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
        "NativePrograms/AutoHost-Rolling.md",
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
    m_options.emplace_back(&TROLL_HOSTING, "TROLL_HOSTING");
    m_options.emplace_back(&ALTERNATE_GAMES, "ALTERNATE_GAMES");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_internet_settings, "");
    m_options.emplace_back(&CONNECT_TO_INTERNET_DELAY, "CONNECT_TO_INTERNET_DELAY");
    m_options.emplace_back(&ENTER_ONLINE_DEN_DELAY, "ENTER_ONLINE_DEN_DELAY");
    m_options.emplace_back(&OPEN_ONLINE_DEN_LOBBY_DELAY, "OPEN_ONLINE_DEN_LOBBY_DELAY");
    m_options.emplace_back(&RAID_START_TO_EXIT_DELAY, "RAID_START_TO_EXIT_DELAY");
    m_options.emplace_back(&DELAY_TO_SELECT_MOVE, "DELAY_TO_SELECT_MOVE");
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

    grip_menu_connect_go_home(env.console);

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
        enter_den(env.console, ENTER_ONLINE_DEN_DELAY, SKIPS != 0, HOST_ONLINE);

        //  Don't delay if it's the first iteration.
        if (first){
            first = false;
        }else{
            pbf_wait(env.console, EXTRA_DELAY_BETWEEN_RAIDS);
        }

        uint8_t code[8];
        if (RAID_CODE.get_code(code)){
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
        enter_lobby(env.console, OPEN_ONLINE_DEN_LOBBY_DELAY, HOST_ONLINE, CATCHABILITY);

        //  Accept friend requests while we wait.
        RaidLobbyState raid_state = raid_lobby_wait(
            env.console, env.logger(),
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

            BlackScreenDetector black_screen(env.console);
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







