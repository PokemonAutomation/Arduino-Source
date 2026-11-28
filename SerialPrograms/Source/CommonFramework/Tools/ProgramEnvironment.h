/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramEnvironment_H
#define PokemonAutomation_ProgramEnvironment_H

#include "Common/Compiler.h"
#include "Common/Cpp/Logging/AbstractLogger.h"

namespace PokemonAutomation{

class StatsTracker;
class ProgramSession;
struct ProgramInfo;



struct ProgramEnvironmentData;

//  It also has the log methods of `FormattedLogMethods`, which pass the line to the
//  program's logger, e.g. `env.info("Found {} eggs", eggs);`.
class ProgramEnvironment : public FormattedLogMethods<ProgramEnvironment>{
public:
    virtual ~ProgramEnvironment() = default;
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
    void log(const std::string& msg, LogLevel level){ m_logger.log(msg, level); }
    void log(const std::string& msg, Color color, LogLevel level){ m_logger.log(msg, color, level); }

    virtual void log_to_ui(const std::string& msg, Color color = Color()){}


public:
    //  Stats Management

    void update_stats();

    template <typename StatsType>
    StatsType& current_stats();
    const StatsTracker* current_stats() const{ return m_current_stats; }

    const StatsTracker* historical_stats() const{ return m_historical_stats; }
    std::string historical_stats_str() const;


private:
    const ProgramInfo& m_program_info;
    ProgramSession& m_session;
    Logger& m_logger;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;
};




//  Templates

template <typename StatsType>
StatsType& ProgramEnvironment::current_stats(){
    return *static_cast<StatsType*>(m_current_stats);
}




}
#endif

