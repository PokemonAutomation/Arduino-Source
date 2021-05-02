/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/Qt/CodeValidator.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_TradeBot.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

TradeBot::TradeBot()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Trade Bot",
        "NativePrograms/TradeBot.md",
        "Surprise trade with a code for hosting giveaways."
    )
    , TRADE_CODE(
        "<b>Trade Code:</b>",
        8,
        "1280 0000"
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        2
    )
    , LINK_TRADE_EXTRA_LINE(
        "<b>Link Trade Extra Line:</b><br>Set this if you are playing in German.",
        false
    )
    , SEARCH_DELAY(
        "<b>Time to wait for a Trade Partner:</b>",
        "20 * TICKS_PER_SECOND"
    )
    , CONFIRM_DELAY(
        "<b>Time to wait for Partner to Confirm:</b>",
        "10 * TICKS_PER_SECOND"
    )
    , TRADE_START(
        "<b>Time for Trade to Start:</b>",
        "10 * TICKS_PER_SECOND"
    )
    , TRADE_COMMUNICATION(
        "<b>Communication Window:</b>",
        "20 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , TRADE_ANIMATION(
        "<b>Trade Animation Time:</b>",
        "23 * TICKS_PER_SECOND"
    )
    , EVOLVE_DELAY(
        "<b>Evolve Delay:</b>",
        "30 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&TRADE_CODE, "TRADE_CODE");
    m_options.emplace_back(&BOXES_TO_TRADE, "BOXES_TO_TRADE");
    m_options.emplace_back(&LINK_TRADE_EXTRA_LINE, "EXTRA_LINE");
    m_options.emplace_back(&SEARCH_DELAY, "SEARCH_DELAY");
    m_options.emplace_back(&CONFIRM_DELAY, "CONFIRM_DELAY");
    m_options.emplace_back(&TRADE_START, "TRADE_START");
    m_options.emplace_back(&TRADE_COMMUNICATION, "TRADE_COMMUNICATION");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&TRADE_ANIMATION, "TRADE_ANIMATION");
    m_options.emplace_back(&EVOLVE_DELAY, "EVOLVE_DELAY");
}


void TradeBot::trade_slot(const BotBaseContext& context, const uint8_t code[8], uint8_t slot) const{
    ssf_press_button2(context, BUTTON_Y, OPEN_YCOMM_DELAY, 50);
    ssf_press_button2(context, BUTTON_A, 150, 20);
    ssf_press_dpad1(context, DPAD_DOWN, 10);
    ssf_press_button2(context, BUTTON_A, 200, 20);
    if (LINK_TRADE_EXTRA_LINE){
        ssf_press_button2(context, BUTTON_B, 50, 20);
    }
    ssf_press_button2(context, BUTTON_B, 200, 20);
    ssf_press_dpad1(context, DPAD_UP, 10);
    ssf_press_button1(context, BUTTON_A, 5);
    ssf_press_button1(context, BUTTON_B, 5);

    enter_digits(context, 8, code);
    ssf_press_button1(context, BUTTON_PLUS, 200);
    ssf_press_button2(context, BUTTON_B, 125, 10);
    ssf_press_button2(context, BUTTON_A, 50, 10);
    pbf_mash_button(context, BUTTON_B, 400);

    pbf_wait(context, SEARCH_DELAY);

    //  If we're not in a trade, enter Y-COMM to avoid a connection at this point.
    ssf_press_button2(context, BUTTON_Y, OPEN_YCOMM_DELAY, 50);
    ssf_press_button2(context, BUTTON_A, 200, 20);
    ssf_press_button2(context, BUTTON_B, 80, 10);

    //  Move to slot
    while (slot >= 6){
        ssf_press_dpad1(context, DPAD_DOWN, BOX_SCROLL_DELAY);
        slot -= 6;
    }
    while (slot > 0){
        ssf_press_dpad1(context, DPAD_RIGHT, BOX_SCROLL_DELAY);
        slot--;
    }

    //  Select Pokemon
    ssf_press_button1(context, BUTTON_A, 100);
    ssf_press_button1(context, BUTTON_A, CONFIRM_DELAY);

    //  Start Trade
    ssf_press_button1(context, BUTTON_A, TRADE_START);

    //  Cancel out
    for (uint16_t c = 0; c < TRADE_COMMUNICATION + TRADE_ANIMATION; c += 300){
        ssf_press_button1(context, BUTTON_B, 100);
        ssf_press_button1(context, BUTTON_B, 100);
        ssf_press_button1(context, BUTTON_A, 100);
    }
}

void TradeBot::program(SingleSwitchProgramEnvironment& env) const{
    uint8_t code[8];
    TRADE_CODE.to_str(code);

    grip_menu_connect_go_home(env.console);
    resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        for (uint8_t c = 0; c < 30; c++){
            trade_slot(env.console, code, c);
        }

        //  If the previous trade isn't done, either wait to finish or cancel it.
        //  Two iterations of this will merge all the following states:
        //      1.  The last trade finished successfully. Both iterations are no-ops.
        //      2.  The last trade was slightly too slow. The 1st iteration
        //          will finish it. The 2nd iteration is a no-op.
        //      3.  The last trade was very slow. The 1st iteration is suppressed
        //          because the trade is in progress. The 2nd iteration finishes it.
        //      4.  No partner was ever found. The 1st iteration will cancel the trade.
        for (uint8_t c = 0; c < 2; c++){
            ssf_press_button1(env.console, BUTTON_Y, 250);
            ssf_press_button1(env.console, BUTTON_A, 280);
            ssf_press_button1(env.console, BUTTON_B, 280);
            ssf_press_button1(env.console, BUTTON_B, 200);
            ssf_press_button1(env.console, BUTTON_A, 100);
            pbf_mash_button(env.console, BUTTON_B, TRADE_ANIMATION);
        }

        //  Wait out any new pokedex entries or trade evolutions.
        pbf_mash_button(env.console, BUTTON_B, EVOLVE_DELAY);

        //  Change boxes.
        ssf_press_button2(env.console, BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
        ssf_press_button2(env.console, BUTTON_A, MENU_TO_POKEMON_DELAY, 10);
        ssf_press_button2(env.console, BUTTON_R, POKEMON_TO_BOX_DELAY, 10);
        ssf_press_button2(env.console, BUTTON_R, BOX_CHANGE_DELAY, 10);
        pbf_mash_button(env.console, BUTTON_B, BOX_TO_POKEMON_DELAY + POKEMON_TO_MENU_DELAY + OVERWORLD_TO_MENU_DELAY);
    }

    ssf_press_button2(env.console, BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);

    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}







