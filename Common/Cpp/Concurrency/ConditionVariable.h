/*  Condition Variable
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Concurrency_ConditionVariable_H
#define PokemonAutomation_Concurrency_ConditionVariable_H

#include <mutex>
#include "Qt6.9ThreadBugWorkaround.h"

#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND

#include <QWaitCondition>
namespace PokemonAutomation{

class ConditionVariable{
public:
    void notify_one(){
        m_cv.notify_one();
    }
    void notify_all(){
        m_cv.notify_all();
    }

    void wait(std::unique_lock<QMutex>& lock){
        m_cv.wait(lock.mutex());
    }
    template <class Predicate>
    void wait(std::unique_lock<QMutex>& lock, Predicate pred){
        while (!pred()){
            m_cv.wait(lock.mutex());
        }
    }

    template <class Rep, class Period>
    void wait_for(
        std::unique_lock<QMutex>& lock,
        const std::chrono::duration<Rep, Period>& rel_time
    ){
        m_cv.wait(lock.mutex(), rel_time);
    }
    template <class Rep, class Period, class Predicate>
    void wait_for(
        std::unique_lock<QMutex>& lock,
        const std::chrono::duration<Rep, Period>& rel_time,
        Predicate pred
    ){
        auto abs_time = std::chrono::system_clock::now() + rel_time;
        while (!pred()){
            m_cv.wait(lock.mutex(), abs_time - std::chrono::system_clock::now());
        }
    }

    template <class Clock, class Duration>
    void wait_until(
        std::unique_lock<QMutex>& lock,
        const std::chrono::time_point<Clock, Duration>& abs_time
    ){
        m_cv.wait(lock.mutex(), abs_time - std::chrono::system_clock::now());
    }
    template <class Clock, class Duration, class Predicate>
    void wait_until(
        std::unique_lock<QMutex>& lock,
        const std::chrono::time_point<Clock, Duration>& abs_time,
        Predicate pred
    ){
        while (!pred()){
            m_cv.wait(lock.mutex(), abs_time - std::chrono::system_clock::now());
        }
    }


private:
    QWaitCondition m_cv;
};

}

#else

#include <condition_variable>
namespace PokemonAutomation{
    using ConditionVariable = std::condition_variable;
}

#endif

#endif
