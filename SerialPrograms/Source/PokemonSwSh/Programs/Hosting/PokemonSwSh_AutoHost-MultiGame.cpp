/*  Multi-Game Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_LobbyWait.h"
#include "PokemonSwSh_AutoHost-MultiGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

AutoHostMultiGame::AutoHostMultiGame()
    : SingleSwitchProgram(
        FeedbackType::OPTIONAL_, PABotBaseLevel::PABOTBASE_12KB,
        "Auto-Host Multi-Game",
        "NativePrograms/AutoHost-MultiGame.md",
        "Run AutoHost-Rolling across multiple game saves. (Up to 16 dens!)"
    )
    , HOST_ONLINE("<b>Host Online:</b>", true)
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        "60 * TICKS_PER_SECOND"
    )
    , FR_FORWARD_ACCEPT(
        "<b>Forward Friend Accept:</b><br>Accept FRs this many raids in the future.",
        1
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
    m_options.emplace_back(&HOST_ONLINE, "HOST_ONLINE");
    m_options.emplace_back(&LOBBY_WAIT_DELAY, "LOBBY_WAIT_DELAY");
    m_options.emplace_back(&GAME_LIST, "GAME_LIST");
    m_options.emplace_back(&FR_FORWARD_ACCEPT, "FR_FORWARD_ACCEPT");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_internet_settings, "");
    m_options.emplace_back(&CONNECT_TO_INTERNET_DELAY, "CONNECT_TO_INTERNET_DELAY");
    m_options.emplace_back(&ENTER_ONLINE_DEN_DELAY, "ENTER_ONLINE_DEN_DELAY");
    m_options.emplace_back(&OPEN_ONLINE_DEN_LOBBY_DELAY, "OPEN_ONLINE_DEN_LOBBY_DELAY");
    m_options.emplace_back(&RAID_START_TO_EXIT_DELAY, "RAID_START_TO_EXIT_DELAY");
    m_options.emplace_back(&DELAY_TO_SELECT_MOVE, "DELAY_TO_SELECT_MOVE");
}



void AutoHostMultiGame::run_autohost(
    SingleSwitchProgramEnvironment& env,
    const MultiHostTable::GameSlot& game,
    uint8_t accept_FR_slot,
    uint16_t lobby_wait_delay,
    Catchability catchability
) const{
    roll_den(env.console, ENTER_ONLINE_DEN_DELAY, OPEN_ONLINE_DEN_LOBBY_DELAY, game.skips, catchability);

    if (HOST_ONLINE){
        connect_to_internet(env.console, OPEN_YCOMM_DELAY, CONNECT_TO_INTERNET_DELAY);
    }
    enter_den(env.console, ENTER_ONLINE_DEN_DELAY, game.skips != 0, HOST_ONLINE);

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
    enter_lobby(env.console, OPEN_ONLINE_DEN_LOBBY_DELAY, HOST_ONLINE, catchability);

    //  Accept friend requests while we wait.
    raid_lobby_wait(
        env.console, env.logger(),
        HOST_ONLINE,
        accept_FR_slot,
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
            while (now - start < RAID_START_TO_EXIT_DELAY){
                if (black_screen.black_is_over(env.console.video().snapshot())){
                    env.log("Raid has Started!", "blue");
                    break;
                }
                pbf_mash_button(env.console, BUTTON_A, TICKS_PER_SECOND);
                env.console.botbase().wait_for_all_requests();
                now = system_clock(env.console);
            }
        }
#else
        pbf_mash_button(env.console, BUTTON_A, RAID_START_TO_EXIT_DELAY);
#endif

    //  Select a move.
    if (game.move_slot > 0){
        pbf_wait(env.console, DELAY_TO_SELECT_MOVE);
        pbf_press_button(env.console, BUTTON_A, 20, 80);
        if (game.dynamax){
            pbf_press_dpad(env.console, DPAD_LEFT, 20, 30);
            pbf_press_button(env.console, BUTTON_A, 20, 60);
        }
        for (uint8_t c = 1; c < game.move_slot; c++){
            pbf_press_dpad(env.console, DPAD_DOWN, 20, 30);
        }
        pbf_press_button(env.console, BUTTON_A, 20, 80);
        pbf_press_button(env.console, BUTTON_A, 20, 980);
    }
}

void AutoHostMultiGame::program(SingleSwitchProgramEnvironment& env) const{
    uint16_t start_raid_delay = HOST_ONLINE
        ? OPEN_ONLINE_DEN_LOBBY_DELAY
        : OPEN_LOCAL_DEN_LOBBY_DELAY;
    const uint16_t lobby_wait_delay = LOBBY_WAIT_DELAY < start_raid_delay
        ? 0
        : LOBBY_WAIT_DELAY - start_raid_delay;

    //  Scan den list for any rolling dens.
    bool enable_touch = true;
    for (uint8_t index = 0; index < GAME_LIST.size(); index++){
        const MultiHostTable::GameSlot& game = GAME_LIST[index];
//        if (game.user_slot == 0){
//            break;
//        }
        if (game.skips > 0){
            enable_touch = false;
            break;
        }
    }

    grip_menu_connect_go_home(env.console);

    uint32_t last_touch = 0;
    if (enable_touch && TOUCH_DATE_INTERVAL > 0){
        touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
        last_touch = system_clock(env.console);
    }

    uint32_t raids = 0;
    bool game_slot_flipped = false;
    while (true){
        env.log("Beginning from start of game list.");
        for (uint8_t index = 0; index < GAME_LIST.size(); index++){
            const MultiHostTable::GameSlot& game = GAME_LIST[index];
//            if (game.user_slot == 0){
//                break;
//            }

            env.log("Raids Completed: " + tostr_u_commas(raids++));

            //  Start game.
            rollback_date_from_home(env.console, game.skips);

            //  Sanitize game slot.
            uint8_t game_slot = game.game_slot;
            if (game_slot > 2){
                game_slot = 0;
            }

            //  Calculate current game slot.
            switch (game_slot){
            case 0:
                break;
            case 1:
                game_slot = game_slot_flipped ? 2 : 0;
                break;
            case 2:
                game_slot = game_slot_flipped ? 0 : 2;
                break;
            }
            start_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
                game_slot,
                game.user_slot,
                game.backup_save
            );
            if (game_slot == 2){
                game_slot_flipped = !game_slot_flipped;
            }

            //  Calculate accept_FR_slot.
            size_t FR_index = index;
            for (uint8_t c = 0; c < FR_FORWARD_ACCEPT; c++){
                FR_index++;
                if (FR_index >= GAME_LIST.size()){
                    FR_index = 0;
                }
            }

            //  Run auto-host.
            const MultiHostTable::GameSlot& fr_game = GAME_LIST[FR_index];
            run_autohost(
                env,
                game,
                fr_game.accept_FRs ? fr_game.user_slot : 0,
                lobby_wait_delay,
                game.always_catchable ? ALWAYS_CATCHABLE : MAYBE_UNCATCHABLE
            );

            //  Exit game.
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            close_game(env.console);

            //  Post-raid delay.
            pbf_wait(env.console, parse_ticks_i32(game.post_raid_delay));

            //  Touch the date.
            if (enable_touch && TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
                touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
                last_touch += TOUCH_DATE_INTERVAL;
            }
        }
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

