/*  Multi-Game Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_AutoHostStats.h"
#include "PokemonSwSh_AutoHost.h"
#include "PokemonSwSh_AutoHost-MultiGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


AutoHostMultiGame_Descriptor::AutoHostMultiGame_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:AutoHostMultiGame",
        STRING_POKEMON + " SwSh", "Auto-Host Multi-Game",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/AutoHost-MultiGame.md",
        "Run AutoHost-Rolling across multiple game saves. (Up to 16 dens!)",
        FeedbackType::OPTIONAL_, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
std::unique_ptr<StatsTracker> AutoHostMultiGame_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new AutoHostStats());
}



AutoHostMultiGame::AutoHostMultiGame()
    : RAID_CODE(8)
    , HOST_ONLINE("<b>Host Online:</b>", true)
    , LOBBY_WAIT_DELAY(
        TICKS_PER_SECOND,
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        "60 * TICKS_PER_SECOND"
    )
    , FR_FORWARD_ACCEPT(
        "<b>Forward Friend Accept:</b><br>Accept FRs this many raids in the future.",
        1
    )
    , HOSTING_NOTIFICATIONS("Live-Hosting Announcements", false)
    , NOTIFICATIONS({
        &HOSTING_NOTIFICATIONS.NOTIFICATION,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_internet_settings(
        "<font size=4><b>Internet Settings:</b> Increase these if your internet is slow.</font>"
    )
    , CONNECT_TO_INTERNET_DELAY(
        TICKS_PER_SECOND,
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to enter den.",
        "20 * TICKS_PER_SECOND"
    )
    , ENTER_ONLINE_DEN_DELAY(
        TICKS_PER_SECOND,
        "<b>Enter Online Den Delay:</b><br>\"Communicating\" when entering den while online.",
        "8 * TICKS_PER_SECOND"
    )
    , OPEN_ONLINE_DEN_LOBBY_DELAY(
        TICKS_PER_SECOND,
        "<b>Open Online Den Delay:</b><br>Delay from \"Invite Others\" to when the clock starts ticking.",
        "8 * TICKS_PER_SECOND"
    )
    , RAID_START_TO_EXIT_DELAY(
        TICKS_PER_SECOND,
        "<b>Raid Start to Exit Delay:</b><br>Time from start raid to reset. (when not selecting move)",
        "15 * TICKS_PER_SECOND"
    )
    , DELAY_TO_SELECT_MOVE(
        TICKS_PER_SECOND,
        "<b>Delay to Select Move:</b><br>This + above = time from start raid to select move.",
        "32 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(HOST_ONLINE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(GAME_LIST);
    PA_ADD_OPTION(FR_FORWARD_ACCEPT);
    PA_ADD_OPTION(HOSTING_NOTIFICATIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_OPTION(m_internet_settings);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY);
    PA_ADD_OPTION(ENTER_ONLINE_DEN_DELAY);
    PA_ADD_OPTION(OPEN_ONLINE_DEN_LOBBY_DELAY);
    PA_ADD_OPTION(RAID_START_TO_EXIT_DELAY);
    PA_ADD_OPTION(DELAY_TO_SELECT_MOVE);
}




void AutoHostMultiGame::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    uint16_t start_raid_delay = HOST_ONLINE
        ? OPEN_ONLINE_DEN_LOBBY_DELAY
        : GameSettings::instance().OPEN_LOCAL_DEN_LOBBY_DELAY;
    const uint16_t lobby_wait_delay = LOBBY_WAIT_DELAY < start_raid_delay
        ? 0
        : LOBBY_WAIT_DELAY - start_raid_delay;

    //  Scan den list for any rolling dens.
    bool enable_touch = true;
    for (uint8_t index = 0; index < GAME_LIST.size(); index++){
        const MultiHostSlot& game = GAME_LIST[index];
//        if (game.user_slot == 0){
//            break;
//        }
        if (game.skips > 0){
            enable_touch = false;
            break;
        }
    }

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    if (enable_touch){
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(context);
    }

    uint32_t raids = 0;
    bool game_slot_flipped = false;
    while (true){
        env.log("Beginning from start of game list.");
        for (uint8_t index = 0; index < GAME_LIST.size(); index++){
            env.update_stats();

            const MultiHostSlot& game = GAME_LIST[index];
//            if (game.user_slot == 0){
//                break;
//            }

            env.log("Raids Attempted: " + tostr_u_commas(raids++));

            //  Start game.
            rollback_date_from_home(context, game.skips);

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
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
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
            const MultiHostSlot& fr_game = GAME_LIST[FR_index];
            run_autohost(
                env, env.console, context,
                game.always_catchable
                    ? Catchability::ALWAYS_CATCHABLE
                    : Catchability::MAYBE_UNCATCHABLE,
                game.skips,
                game.use_raid_code ? &RAID_CODE : nullptr, lobby_wait_delay,
                HOST_ONLINE, fr_game.accept_FRs ? fr_game.user_slot : 0,
                game.move_slot, game.dynamax, 0,
                HOSTING_NOTIFICATIONS,
                CONNECT_TO_INTERNET_DELAY,
                ENTER_ONLINE_DEN_DELAY,
                OPEN_ONLINE_DEN_LOBBY_DELAY,
                RAID_START_TO_EXIT_DELAY,
                DELAY_TO_SELECT_MOVE
            );

            //  Exit game.
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            close_game(context);

            //  Post-raid delay.
            pbf_wait(context, parse_ticks_i32(game.post_raid_delay));

            //  Touch the date.
            if (enable_touch){
                TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(context);
            }
        }
    }
}



}
}
}

