/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLZA_TradeRoutines.h"
#include "PokemonLZA_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;




SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonLZA:SelfBoxTrade",
        STRING_POKEMON + " LZA", "Self Box Trade",
        "Programs/PokemonLZA/SelfBoxTrade.html",
        "Trade boxes across two Switches.",
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
#if 0
    : LANGUAGE_LEFT(
        "<b>Game Language of Left Switch:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , LANGUAGE_RIGHT(
        "<b>Game Language of Right Switch:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
#endif
    : BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
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
#if 0
    , SKIP_TRADE_EVOLUTIONS(
        "<b>Skip Trade Evolutions:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
#endif
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
//    PA_ADD_OPTION(LANGUAGE_LEFT);
//    PA_ADD_OPTION(LANGUAGE_RIGHT);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(START_ROW);
    PA_ADD_OPTION(START_COL);
//    PA_ADD_OPTION(SKIP_TRADE_EVOLUTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}
void SelfBoxTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    env.run_in_parallel(
        scope,
        [](ConsoleHandle& console, ProControllerContext& context){
            assert_16_9_720p_min(console, console);
        }
    );

    TradeStats& stats = env.current_stats<TradeStats>();
    env.update_stats();

    uint8_t start_row = START_ROW - 1;
    uint8_t start_col = START_COL - 1;

    for (uint8_t box = 0; box < BOXES_TO_TRADE; box++){
        if (box != 0){
            env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
                pbf_press_button(context, BUTTON_R, 60, 100);
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
