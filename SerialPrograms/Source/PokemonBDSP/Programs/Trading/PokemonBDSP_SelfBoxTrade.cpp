/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_TradeRoutines.h"
#include "PokemonBDSP_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonBDSP:SelfBoxTrade",
        STRING_POKEMON + " BDSP", "Self Box Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/SelfBoxTrade.md",
        "Trade boxes of " + STRING_POKEMON + " between two local Switches.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        2, 2, 2
    )
{}
std::unique_ptr<StatsTracker> SelfBoxTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}



SelfBoxTrade::SelfBoxTrade()
    : BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2, 0, 40
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void SelfBoxTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    TradeStats& stats = env.current_stats<TradeStats>();
    env.update_stats();

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        if (box != 0){
            env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
                pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().BOX_CHANGE_DELAY0);
            });
        }
        trade_current_box(env, scope, NOTIFICATION_STATUS_UPDATE, stats);
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
