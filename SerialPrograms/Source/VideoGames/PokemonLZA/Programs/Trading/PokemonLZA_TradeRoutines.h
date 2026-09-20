/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TradeRoutines_H
#define PokemonAutomation_PokemonLZA_TradeRoutines_H

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/MultiConsoleErrors.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



struct TradeStats : public StatsTracker{
    TradeStats();
    std::atomic<uint64_t>& trades;
    std::atomic<uint64_t>& skipped;
    std::atomic<uint64_t>& errors;
};



bool trade_current_pokemon(
    VideoStream& stream, ProControllerContext& context,
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
