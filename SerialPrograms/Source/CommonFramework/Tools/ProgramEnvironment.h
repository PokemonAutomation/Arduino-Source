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
#include "Common/Cpp/Pimpl.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Notifications/ProgramInfo.h"


// Forward declare std mutex and condition_variable
// to reduce the amount of header includes.
namespace std{

// for libc++ used by clang, mutex and condition_variable are
// defined in inline std::__1 namespace instead of std.
#ifdef _LIBCPP_VERSION
    inline namespace __1 {
#endif

    class mutex;
    class condition_variable;

#ifdef _LIBCPP_VERSION
    }
#endif

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
        const ProgramInfo& program_info,
        LoggerQt& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    );

    CancellableScope& scope(){ return m_scope; }
    const ProgramInfo& program_info() const;

    template <class... Args>
    void log(Args&&... args);
    LoggerQt& logger(){ return m_logger; }

    AsyncDispatcher& realtime_dispatcher();
    AsyncDispatcher& inference_dispatcher();

    void update_stats(const std::string& override_current = "");

    template <typename StatsType>
    StatsType& stats();
    const StatsTracker* stats() const{ return m_current_stats; }

    bool is_stopping() const;
    void check_stopping() const;


public:
    //  When the user issues a stop command, signal all the conditional variables
    //  under their associated mutexes.
    void register_stop_program_signal(std::mutex& lock, std::condition_variable& cv);
    void deregister_stop_program_signal(std::condition_variable& cv);


public:
    //  Use these since they will wake up on program stop.

    void wait_for(std::chrono::milliseconds duration);
    void wait_until(std::chrono::system_clock::time_point stop);
    void notify_all();


public:
    //  Don't call this from a program.
    void signal_stop();


signals:
    //  Called internally. You can connect to this.
    void set_status(QString status);


private:
    LoggerQt& m_logger;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;

    CancellableHolder<CancellableScope> m_scope;

    Pimpl<ProgramEnvironmentData> m_data;
};



class ProgramStopNotificationScope{
public:
    ProgramStopNotificationScope(
        ProgramEnvironment& env,
        std::mutex& lock,
        std::condition_variable& cv
    )
        : m_env(env)
        , m_cv(cv)
    {
        env.register_stop_program_signal(lock, cv);
    }
    ~ProgramStopNotificationScope(){
        m_env.deregister_stop_program_signal(m_cv);
    }

private:
    ProgramEnvironment& m_env;
    std::condition_variable& m_cv;
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

