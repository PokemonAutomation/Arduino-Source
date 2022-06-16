/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramEnvironment_H
#define PokemonAutomation_ProgramEnvironment_H

#include <QObject>
#include "Common/Cpp/Pimpl.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Logging/LoggerQt.h"



namespace PokemonAutomation{

class AsyncDispatcher;
class StatsTracker;
struct ProgramInfo;




struct ProgramEnvironmentData;

class ProgramEnvironment : public QObject{
    Q_OBJECT

public:
    ~ProgramEnvironment();
    ProgramEnvironment(
        const ProgramInfo& program_info,
        LoggerQt& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    );

    const ProgramInfo& program_info() const;

    template <class... Args>
    void log(Args&&... args);
    LoggerQt& logger(){ return m_logger; }

    AsyncDispatcher& realtime_dispatcher();
    AsyncDispatcher& inference_dispatcher();

    void update_stats(const std::string& override_current = "");

    template <typename StatsType>
    StatsType& current_stats();
    const StatsTracker* current_stats() const{ return m_current_stats; }

    const StatsTracker* historical_stats() const{ return m_historical_stats; }
    std::string historical_stats_str() const;


signals:
    //  Called internally. You can connect to this.
    void set_status(QString status);


private:
    LoggerQt& m_logger;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;

    Pimpl<ProgramEnvironmentData> m_data;
};




//  Templates


template <class... Args>
void ProgramEnvironment::log(Args&&... args){
    m_logger.log(std::forward<Args>(args)...);
}

template <typename StatsType>
StatsType& ProgramEnvironment::current_stats(){
    return *static_cast<StatsType*>(m_current_stats);
}




}
#endif

