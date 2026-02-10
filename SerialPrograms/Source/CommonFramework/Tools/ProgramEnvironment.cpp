/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "ProgramEnvironment.h"


namespace PokemonAutomation{





ProgramEnvironment::ProgramEnvironment(
    const ProgramInfo& program_info,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
)
    : m_program_info(program_info)
    , m_session(session)
    , m_logger(session.logger())
    , m_current_stats(current_stats)
    , m_historical_stats(historical_stats)
{}

const ProgramInfo& ProgramEnvironment::program_info() const{
    return m_program_info;
}


void ProgramEnvironment::update_stats(){
    m_session.report_stats_changed();
    if (m_current_stats){
        m_logger.log("Current Stats: " + m_current_stats->to_str(StatsTracker::PrintMode::DUMP));
    }
}


std::string ProgramEnvironment::historical_stats_str() const{
    return m_historical_stats ? m_historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN) : "";
}




}
