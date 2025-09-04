/*  Surprise Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
#include "PokemonSwSh_SurpriseTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


SurpriseTrade_Descriptor::SurpriseTrade_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:SurpriseTrade",
        STRING_POKEMON + " SwSh", "Surprise Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/SurpriseTrade.md",
        "Surprise trade away boxes of " + STRING_POKEMON,
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}



SurpriseTrade::SurpriseTrade()
    : BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2
    )
    , INITIAL_WAIT0(
        "<b>Time to wait for a Trade Partner:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "30 s"
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

    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(INITIAL_WAIT0);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(TRADE_ANIMATION0);
    PA_ADD_OPTION(EVOLVE_DELAY0);
}


void SurpriseTrade::trade_slot(ProControllerContext& context, uint8_t slot, bool next_box) const{
    ssf_press_button(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY0, 400ms);
    ssf_press_dpad(context, DPAD_DOWN, 80ms);
    ssf_press_button(context, BUTTON_A, 2240ms, 160ms);

    if (next_box){
        ssf_press_button(context, BUTTON_R, GameSettings::instance().BOX_CHANGE_DELAY0);
    }

    //  Move to slot
    while (slot >= 6){
        box_scroll(context, DPAD_DOWN);
        slot -= 6;
    }
    while (slot > 0){
        box_scroll(context, DPAD_RIGHT);
        slot--;
    }

    ssf_press_button(context, BUTTON_A, 400ms);
    ssf_press_button(context, BUTTON_A, 4000ms);
    ssf_press_button(context, BUTTON_A, 800ms);
    ssf_press_button(context, BUTTON_A, 800ms);

    pbf_mash_button(context, BUTTON_B, INITIAL_WAIT0);

    //  This is a state-merging operation.
    //  If we just finished a trade, this will start the animation for it.
    //  If we failed the previous trade and are stuck in the wrong parity, this
    //  is a no-op that will correct the parity and setup the next trade.
    ssf_press_button(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY0);
    pbf_mash_button(context, BUTTON_B, TRADE_ANIMATION0);
}

void SurpriseTrade::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        //  At this point, we MUST be in the overworld with no pending trade.
        //  Otherwise the box transition will fail. Therefore we add a cleanup
        //  stage after each box to make sure we are in this state.
        trade_slot(context, 0, box != 0);

        for (uint8_t c = 1; c < 30; c++){
            trade_slot(context, c, false);
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
            ssf_press_dpad(context, DPAD_DOWN, 160ms);
            ssf_press_button(context, BUTTON_A, 2240ms);
            ssf_press_button(context, BUTTON_B, 2240ms);
            ssf_press_button(context, BUTTON_B, 1600ms);
            ssf_press_button(context, BUTTON_A, 800ms);
            pbf_mash_button(context, BUTTON_B, TRADE_ANIMATION0);
        }

        //  Wait out any new pokedex entries or trade evolutions.
        pbf_mash_button(context, BUTTON_B, EVOLVE_DELAY0);
    }

    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);
}


}
}
}
