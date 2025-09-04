/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_TradeRoutines.h"
#include "PokemonSV_SelfBoxTrade.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"

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
        2, 0, 32
    )
    , START_ROW(
        "<b>Starting Row of 1st Box:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 5
    )
    , START_COL(
        "<b>Starting Column of 1st Box:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 6
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(START_ROW);
    PA_ADD_OPTION(START_COL);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void SelfBoxTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    TradeStats& stats = env.current_stats<TradeStats>();
    env.update_stats();

    uint8_t start_row = START_ROW - 1;
    uint8_t start_col = START_COL - 1;

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        if (box != 0){
            env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
                move_to_right_box(context);
//                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            });
        }
        trade_current_box(env, scope, NOTIFICATION_STATUS_UPDATE, stats, start_row, start_col);
        start_row = 0;
        start_col = 0;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
