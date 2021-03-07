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
#include "ClientSource/Connection/BotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class ProgramEnvironment : public QObject{
public:
    ProgramEnvironment()
        : m_enable_feedback(true)
        , m_stopping(false)
    {}

    bool is_stopping() const{
        return m_stopping.load(std::memory_order_acquire);
    }

    void check_stopping() const{
        if (is_stopping()){
            throw PokemonAutomation::CancelledException();
        }
    }

    template <typename Duration>
    void wait(Duration duration){
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

    void signal_stop(){
        m_stopping.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }

private:
    std::atomic<bool> m_enable_feedback;
    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
};



}
#endif

