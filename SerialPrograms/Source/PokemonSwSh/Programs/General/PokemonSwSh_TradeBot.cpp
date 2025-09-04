/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
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
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
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
        LockMode::LOCK_WHILE_RUNNING,
        2
    )
    , LINK_TRADE_EXTRA_LINE(
        "<b>Link Trade Extra Line:</b><br>Set this if you are playing in German.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SEARCH_DELAY0(
        "<b>Time to wait for a Trade Partner:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
    , CONFIRM_DELAY0(
        "<b>Time to wait for Partner to Confirm:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
    , TRADE_START0(
        "<b>Time for Trade to Start:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
    , TRADE_COMMUNICATION0(
        "<b>Communication Window:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , TRADE_ANIMATION0(
        "<b>Trade Animation Time:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "23 s"
    )
    , EVOLVE_DELAY0(
        "<b>Evolve Delay:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "30 s"
    )
{
    PA_ADD_OPTION(START_LOCATION);

    PA_ADD_OPTION(TRADE_CODE);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(LINK_TRADE_EXTRA_LINE);
    PA_ADD_OPTION(SEARCH_DELAY0);
    PA_ADD_OPTION(CONFIRM_DELAY0);
    PA_ADD_OPTION(TRADE_START0);
    PA_ADD_OPTION(TRADE_COMMUNICATION0);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(TRADE_ANIMATION0);
    PA_ADD_OPTION(EVOLVE_DELAY0);
}


void TradeBot::trade_slot(
    ConsoleHandle& console, ProControllerContext& context,
    const std::string& code, uint8_t slot
) const{
    ssf_press_button(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY0, 400ms);
    ssf_press_button(context, BUTTON_A, 1200ms, 160ms);
    ssf_press_dpad(context, DPAD_DOWN, 80ms);
    ssf_press_button(context, BUTTON_A, 1600ms, 160ms);
    if (LINK_TRADE_EXTRA_LINE){
        ssf_press_button(context, BUTTON_B, 400ms, 160ms);
    }
    ssf_press_button(context, BUTTON_B, 1600ms, 160ms);
    ssf_press_dpad(context, DPAD_UP, 80ms);
    ssf_press_button(context, BUTTON_A, 40ms);
    ssf_press_button(context, BUTTON_B, 40ms);

    FastCodeEntry::numberpad_enter_code(console, context, code, true);
    ssf_press_button(context, BUTTON_PLUS, 1600ms);
    ssf_press_button(context, BUTTON_B, 1000ms, 80ms);
    ssf_press_button(context, BUTTON_A, 400ms, 80ms);
    pbf_mash_button(context, BUTTON_B, 400);

    pbf_wait(context, SEARCH_DELAY0);

    //  If we're not in a trade, enter Y-COMM to avoid a connection at this point.
    ssf_press_button(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY0, 400ms);
    ssf_press_button(context, BUTTON_A, 1600ms, 160ms);
    ssf_press_button(context, BUTTON_B, 640ms, 80ms);

    //  Move to slot
    while (slot >= 6){
        box_scroll(context, DPAD_DOWN);
        slot -= 6;
    }
    while (slot > 0){
        box_scroll(context, DPAD_RIGHT);
        slot--;
    }

    //  Select Pokemon
    ssf_press_button(context, BUTTON_A, 800ms);
    ssf_press_button(context, BUTTON_A, CONFIRM_DELAY0);

    //  Start Trade
    ssf_press_button(context, BUTTON_A, TRADE_START0);

    //  Cancel out
    for (Milliseconds c = 0ms; c < TRADE_COMMUNICATION0.get() + TRADE_ANIMATION0.get(); c += 2400ms){
        ssf_press_button(context, BUTTON_B, 800ms);
        ssf_press_button(context, BUTTON_B, 800ms);
        ssf_press_button(context, BUTTON_A, 800ms);
    }
}

void TradeBot::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    std::string code = TRADE_CODE.to_str();

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        for (uint8_t c = 0; c < 30; c++){
            trade_slot(env.console, context, code, c);
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
            ssf_press_button(context, BUTTON_Y, 2000ms);
            ssf_press_button(context, BUTTON_A, 2240ms);
            ssf_press_button(context, BUTTON_B, 2240ms);
            ssf_press_button(context, BUTTON_B, 1600ms);
            ssf_press_button(context, BUTTON_A, 800ms);
            pbf_mash_button(context, BUTTON_B, TRADE_ANIMATION0);
        }

        //  Wait out any new pokedex entries or trade evolutions.
        pbf_mash_button(context, BUTTON_B, EVOLVE_DELAY0);

        //  Change boxes.
        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
        ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, 80ms);
        ssf_press_button(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, 80ms);
        ssf_press_button(context, BUTTON_R, GameSettings::instance().BOX_CHANGE_DELAY0, 80ms);
        pbf_mash_button(
            context, BUTTON_B,
            GameSettings::instance().BOX_TO_POKEMON_DELAY0.get() +
            GameSettings::instance().POKEMON_TO_MENU_DELAY0.get() +
            GameSettings::instance().OVERWORLD_TO_MENU_DELAY0.get()
        );
    }

    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);
}


}
}
}







