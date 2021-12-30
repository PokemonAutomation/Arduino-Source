/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramEnvironment_H
#define PokemonAutomation_ProgramEnvironment_H

#include <memory>
#include <chrono>
#include <QObject>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramInfo.h"


namespace std{
    class mutex;
    class condition_variable;
}

namespace PokemonAutomation{

class AsyncDispatcher;
class StatsTracker;




struct ProgramEnvironmentData;

class ProgramEnvironment : public QObject{
    Q_OBJECT

public:
    ~ProgramEnvironment();
    ProgramEnvironment(
        ProgramInfo program_info,
        Logger& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    );

    const ProgramInfo& program_info() const;

    template <class... Args>
    void log(Args&&... args);
    Logger& logger(){ return m_logger; }

    AsyncDispatcher& dispatcher();

    void update_stats(const std::string& override_current = "");

    template <typename StatsType>
    StatsType& stats();

    bool is_stopping() const;
    void check_stopping() const;


public:
    //  Use these since they will wake up on program stop.
    std::mutex& lock();
    std::condition_variable& cv();

    void wait_for(std::chrono::milliseconds duration);
    void notify_all();


public:
    //  Don't call this from a program.
    void signal_stop();


signals:
    //  Called internally. You can connect to this.
    void set_status(QString status);


private:
    Logger& m_logger;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;

    std::unique_ptr<ProgramEnvironmentData> m_data;
};




//  Templates


template <class... Args>
void ProgramEnvironment::log(Args&&... args){
    m_logger.log(std::forward<Args>(args)...);
}

template <typename StatsType>
StatsType& ProgramEnvironment::stats(){
    return *static_cast<StatsType*>(m_current_stats);
}



}
#endif

