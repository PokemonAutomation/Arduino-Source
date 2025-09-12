/*  Controller with Scheduler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This is a helper class for all the common methods for a controller that
 *  uses the SuperscalarScheduler.
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerWithScheduler_H
#define PokemonAutomation_Controllers_ControllerWithScheduler_H

#include <mutex>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/RecursiveThrottler.h"
#include "SuperscalarScheduler.h"

namespace PokemonAutomation{



class ControllerWithScheduler{
public:
    ControllerWithScheduler(Logger& logger)
        : m_logger(logger)
        , m_scheduler(logger, Milliseconds(4))
    {}

    RecursiveThrottler& logging_throttler(){
        return m_logging_throttler;
    }


public:
    //  Superscalar Commands (the "ssf" framework)

    void issue_barrier(const Cancellable* cancellable){
        SuperscalarScheduler::Schedule schedule;
        std::lock_guard<std::mutex> lg0(m_issue_lock);
        {
            std::lock_guard<std::mutex> lg1(m_state_lock);
            m_scheduler.issue_wait_for_all(schedule);
        }
        execute_schedule(cancellable, schedule);
        if (m_logging_throttler){
            m_logger.log("issue_barrier()", COLOR_DARKGREEN);
        }
    }
    void issue_nop(const Cancellable* cancellable, Milliseconds duration){
        SuperscalarScheduler::Schedule schedule;
        std::lock_guard<std::mutex> lg0(m_issue_lock);
        {
            std::lock_guard<std::mutex> lg1(m_state_lock);
            if (cancellable){
                cancellable->throw_if_cancelled();
            }
            m_scheduler.issue_nop(schedule, WallDuration(duration));
        }
        execute_schedule(cancellable, schedule);
        if (m_logging_throttler){
            m_logger.log(
                "issue_nop(): duration = " + std::to_string(duration.count()) + "ms",
                COLOR_DARKGREEN
            );
        }
    }


protected:
    virtual void execute_state(
        const Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) = 0;
    virtual void execute_schedule(
        const Cancellable* cancellable,
        const SuperscalarScheduler::Schedule& schedule
    ){
        for (const SuperscalarScheduler::ScheduleEntry& entry : schedule){
            execute_state(cancellable, entry);
        }
    }


protected:
    Logger& m_logger;
//    std::atomic<size_t> m_logging_suppress;
    RecursiveThrottler m_logging_throttler;

    SuperscalarScheduler m_scheduler;

    //  If you need both of these locks, always acquire "m_issue_lock" first.

    //  This lock makes sure that only one command is issued at a time. It can
    //  be held for long periods of time if the command queue is full.
    std::mutex m_issue_lock;

    //  This lock protects the state/fields of this class and subclasses.
    //  This lock is never held for a long time.
    std::mutex m_state_lock;
};




}
#endif
