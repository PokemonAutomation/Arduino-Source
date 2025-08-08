/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "ProgramEnvironment.h"


namespace PokemonAutomation{


struct ProgramEnvironmentData{
    const ProgramInfo& m_program_info;

    AsyncDispatcher m_realtime_dispatcher;
    AsyncDispatcher m_realtime_inference_dispatcher;

    ProgramEnvironmentData(
        const ProgramInfo& program_info
    )
        : m_program_info(program_info)
        , m_realtime_dispatcher(
            [](){
                GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(global_logger_tagged());
            },
            0
        )
        , m_realtime_inference_dispatcher(
            [](){
                GlobalSettings::instance().PERFORMANCE->INFERENCE_PIVOT_PRIORITY.set_on_this_thread(global_logger_tagged());
            },
            0
        )
    {}
};




ProgramEnvironment::~ProgramEnvironment(){}

ProgramEnvironment::ProgramEnvironment(
    const ProgramInfo& program_info,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
)
    : m_session(session)
    , m_logger(session.logger())
    , m_current_stats(current_stats)
    , m_historical_stats(historical_stats)
    , m_data(CONSTRUCT_TOKEN, program_info)
{}

const ProgramInfo& ProgramEnvironment::program_info() const{
    return m_data->m_program_info;
}
AsyncDispatcher& ProgramEnvironment::realtime_dispatcher(){
    return m_data->m_realtime_dispatcher;
}
AsyncDispatcher& ProgramEnvironment::realtime_inference_dispatcher(){
    return m_data->m_realtime_inference_dispatcher;
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
