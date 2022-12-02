/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TradeRoutines_H
#define PokemonAutomation_PokemonSV_TradeRoutines_H

#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/MultiConsoleErrors.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



struct TradeStats : public StatsTracker{
    TradeStats();
    std::atomic<uint64_t>& m_trades;
    std::atomic<uint64_t>& m_errors;
};




void trade_current_pokemon(
    ConsoleHandle& console, BotBaseContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
);
void trade_current_box(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    EventNotificationOption& notifications,
    TradeStats& stats,
    uint8_t start_row, uint8_t start_col
);




}
}
}
#endif
