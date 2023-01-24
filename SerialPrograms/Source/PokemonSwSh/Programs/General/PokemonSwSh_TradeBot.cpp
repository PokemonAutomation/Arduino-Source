/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_TradeBot.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


TradeBot_Descriptor::TradeBot_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:TradeBot",
        STRING_POKEMON + " SwSh", "Trade Bot",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/TradeBot.md",
        "Surprise trade with a code for hosting giveaways.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



TradeBot::TradeBot()
    : TRADE_CODE(
        "<b>Trade Code:</b>",
        8,
        "1280 0000"
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        LockWhileRunning::LOCKED,
        2
    )
    , LINK_TRADE_EXTRA_LINE(
        "<b>Link Trade Extra Line:</b><br>Set this if you are playing in German.",
        LockWhileRunning::LOCKED,
        false
    )
    , SEARCH_DELAY(
        "<b>Time to wait for a Trade Partner:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "20 * TICKS_PER_SECOND"
    )
    , CONFIRM_DELAY(
        "<b>Time to wait for Partner to Confirm:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
    , TRADE_START(
        "<b>Time for Trade to Start:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
    , TRADE_COMMUNICATION(
        "<b>Communication Window:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "20 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , TRADE_ANIMATION(
        "<b>Trade Animation Time:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "23 * TICKS_PER_SECOND"
    )
    , EVOLVE_DELAY(
        "<b>Evolve Delay:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "30 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);

    PA_ADD_OPTION(TRADE_CODE);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(LINK_TRADE_EXTRA_LINE);
    PA_ADD_OPTION(SEARCH_DELAY);
    PA_ADD_OPTION(CONFIRM_DELAY);
    PA_ADD_OPTION(TRADE_START);
    PA_ADD_OPTION(TRADE_COMMUNICATION);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(TRADE_ANIMATION);
    PA_ADD_OPTION(EVOLVE_DELAY);
}


void TradeBot::trade_slot(BotBaseContext& context, const uint8_t code[8], uint8_t slot) const{
    ssf_press_button2(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY, 50);
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
    ssf_press_button2(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY, 50);
    ssf_press_button2(context, BUTTON_A, 200, 20);
    ssf_press_button2(context, BUTTON_B, 80, 10);

    //  Move to slot
    while (slot >= 6){
        ssf_press_dpad1(context, DPAD_DOWN, GameSettings::instance().BOX_SCROLL_DELAY);
        slot -= 6;
    }
    while (slot > 0){
        ssf_press_dpad1(context, DPAD_RIGHT, GameSettings::instance().BOX_SCROLL_DELAY);
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

void TradeBot::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    uint8_t code[8];
    TRADE_CODE.to_str(code);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        for (uint8_t c = 0; c < 30; c++){
            trade_slot(context, code, c);
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
            ssf_press_button1(context, BUTTON_Y, 250);
            ssf_press_button1(context, BUTTON_A, 280);
            ssf_press_button1(context, BUTTON_B, 280);
            ssf_press_button1(context, BUTTON_B, 200);
            ssf_press_button1(context, BUTTON_A, 100);
            pbf_mash_button(context, BUTTON_B, TRADE_ANIMATION);
        }

        //  Wait out any new pokedex entries or trade evolutions.
        pbf_mash_button(context, BUTTON_B, EVOLVE_DELAY);

        //  Change boxes.
        ssf_press_button2(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY, 20);
        ssf_press_button2(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY, 10);
        ssf_press_button2(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY, 10);
        ssf_press_button2(context, BUTTON_R, GameSettings::instance().BOX_CHANGE_DELAY, 10);
        pbf_mash_button(
            context, BUTTON_B,
            GameSettings::instance().BOX_TO_POKEMON_DELAY +
            GameSettings::instance().POKEMON_TO_MENU_DELAY +
            GameSettings::instance().OVERWORLD_TO_MENU_DELAY
        );
    }

    ssf_press_button2(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);
}


}
}
}







