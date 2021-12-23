/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_TradeRoutines_H
#define PokemonAutomation_PokemonBDSP_TradeRoutines_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MultiConsoleErrorState{
public:
    void report_unrecoverable_error(Logger& logger, const std::string& msg);
    void check_unrecoverable_error(Logger& logger);

private:
    std::atomic<bool> m_unrecoverable_error;
};


struct TradeStats : public StatsTracker{
    TradeStats();
    std::atomic<uint64_t>& m_trades;
    std::atomic<uint64_t>& m_errors;
};




void trade_current_pokemon(
    ProgramEnvironment& env, ConsoleHandle& console,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
);
void trade_current_box(
    MultiSwitchProgramEnvironment& env,
    EventNotificationOption& notifications,
    TradeStats& stats
);




}
}
}
#endif
