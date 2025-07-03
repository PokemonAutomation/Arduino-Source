/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramEnvironment_H
#define PokemonAutomation_ProgramEnvironment_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{

class AsyncDispatcher;
class StatsTracker;
class ProgramSession;
struct ProgramInfo;



struct ProgramEnvironmentData;

class ProgramEnvironment{
public:
    ~ProgramEnvironment();
    ProgramEnvironment(
        const ProgramInfo& program_info,
        ProgramSession& session,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    );

    const ProgramInfo& program_info() const;

public:
    //  Logging
    Logger& logger(){ return m_logger; }
    void log(const char* msg, Color color = Color()){ m_logger.log(msg, color); }
    void log(const std::string& msg, Color color = Color()){ m_logger.log(msg, color); }

public:
    //  Thread Pools

    //  A high-priority dispatcher for program and logic threads that cannot
    //  tolerate being delayed.
    AsyncDispatcher& realtime_dispatcher();

    //  A high-priority dispatcher for inference where starvation may cause the
    //  program to encounter issues.
    AsyncDispatcher& realtime_inference_dispatcher();

public:
    //  Stats Management

    void update_stats();

    template <typename StatsType>
    StatsType& current_stats();
    const StatsTracker* current_stats() const{ return m_current_stats; }

    const StatsTracker* historical_stats() const{ return m_historical_stats; }
    std::string historical_stats_str() const;


private:
    ProgramSession& m_session;
    Logger& m_logger;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;

    Pimpl<ProgramEnvironmentData> m_data;
};




//  Templates

template <typename StatsType>
StatsType& ProgramEnvironment::current_stats(){
    return *static_cast<StatsType*>(m_current_stats);
}




}
#endif

