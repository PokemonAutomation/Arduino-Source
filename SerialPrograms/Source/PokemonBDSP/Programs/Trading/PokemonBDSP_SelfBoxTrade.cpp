/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_TradeRoutines.h"
#include "PokemonBDSP_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonBDSP:SelfBoxTrade",
        STRING_POKEMON + " BDSP", "Self Box Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/SelfBoxTrade.md",
        "Trade boxes of " + STRING_POKEMON + " between two local Switches.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 2, 2
    )
{}

SelfBoxTrade::SelfBoxTrade(const SelfBoxTrade_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        2, 0, 40
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



std::unique_ptr<StatsTracker> SelfBoxTrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}



void SelfBoxTrade::program(MultiSwitchProgramEnvironment& env){
    TradeStats& stats = env.stats<TradeStats>();
    env.update_stats();

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        if (box != 0){
            env.run_in_parallel([](ConsoleHandle& console){
                pbf_press_button(console, BUTTON_R, 20, GameSettings::instance().BOX_CHANGE_DELAY_0);
            });
        }
        trade_current_box(env, NOTIFICATION_STATUS_UPDATE, stats);
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}




}
}
}
