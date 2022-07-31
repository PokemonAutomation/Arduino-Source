/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_TradeRoutines.h"
#include "PokemonBDSP_SelfTouchTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


SelfTouchTrade_Descriptor::SelfTouchTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonBDSP:SelfTouchTrade",
        STRING_POKEMON + " BDSP", "Self Touch Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/SelfTouchTrade.md",
        "Touch trade boxes of " + STRING_POKEMON + " between two local Switches.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 2, 2
    )
{}
std::unique_ptr<StatsTracker> SelfTouchTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}


SelfTouchTrade::SelfTouchTrade(const SelfTouchTrade_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , HOSTING_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch hosting the " + STRING_POKEMON + " to be touch-traded to the other.",
        {
            "Switch 0 (Left)",
            "Switch 1 (Right)",
        },
        0
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Touch-Trade:</b>",
        2, 0, 40
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(HOSTING_SWITCH);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void SelfTouchTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    TradeStats& stats = env.current_stats<TradeStats>();
    env.update_stats();

    BotBaseContext host(scope, env.consoles[HOSTING_SWITCH].botbase());

    //  Swap trade all the boxes.
    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        if (box != 0){
            pbf_press_button(host, BUTTON_R, 20, GameSettings::instance().BOX_CHANGE_DELAY_0);
        }
        trade_current_box(env, scope, NOTIFICATION_STATUS_UPDATE, stats);
    }

    //  Trade back the last box.
    for (uint8_t box = 1; box < BOXES_TO_TRADE; box++){
        pbf_press_button(host, BUTTON_L, 20, GameSettings::instance().BOX_CHANGE_DELAY_0);
    }
    trade_current_box(env, scope, NOTIFICATION_STATUS_UPDATE, stats);

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
