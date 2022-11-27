/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_TradeRoutines.h"
#include "PokemonSV_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
    using namespace Pokemon;


SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:SelfBoxTrade",
        STRING_POKEMON + " SV", "Self Box Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/SelfBoxTrade.md",
        "Trade boxes of " + STRING_POKEMON + " between two local Switches.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 2, 2
    )
{}
std::unique_ptr<StatsTracker> SelfBoxTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}



SelfBoxTrade::SelfBoxTrade()
    : BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        LockWhileRunning::LOCKED,
        2, 0, 32
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
            env.run_in_parallel(scope, [](ConsoleHandle& console, BotBaseContext& context){
                pbf_press_button(context, BUTTON_R, 20, 105);
//                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
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
