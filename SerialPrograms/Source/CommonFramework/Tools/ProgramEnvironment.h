/*  Program Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramEnvironment_H
#define PokemonAutomation_ProgramEnvironment_H

#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <QObject>
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"
#include "Logger.h"
#include "StatsTracking.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class ProgramEnvironment : public QObject{
    Q_OBJECT

public:
    ProgramEnvironment(
//        std::string program_identifier,
        Logger& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    );

//    const std::string& program_identifier(){ return m_program_identifier; }

    template <class... Args>
    void log(Args&&... args);
    Logger& logger(){ return m_logger; }

    AsyncDispatcher& dispatcher(){ return m_dispatcher; }

    void update_stats(const std::string& override_current = "");

    template <typename StatsType>
    StatsType& stats();

    bool is_stopping() const;
    void check_stopping() const;


public:
    //  Use these since they will wake up on program stop.
    std::mutex& lock(){ return m_lock; }
    std::condition_variable& cv(){ return m_cv; }

    template <typename Duration>
    void wait_for(Duration duration);
    void notify_all();


public:
    //  Don't call this from a program.
    void signal_stop();


signals:
    void set_status(QString status);


private:
    std::atomic<bool> m_enable_feedback;
    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;

//    std::string m_program_identifier;
    Logger& m_logger;

    AsyncDispatcher m_dispatcher;

    StatsTracker* m_current_stats;
    const StatsTracker* m_historical_stats;
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

template <typename Duration>
void ProgramEnvironment::wait_for(Duration duration){
    check_stopping();

    auto start = std::chrono::system_clock::now();
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait_for(
        lg, duration,
        [=]{
            return std::chrono::system_clock::now() - start >= duration || is_stopping();
        }
    );

    check_stopping();
}


}
#endif

