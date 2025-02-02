/*  Superscalar Scheduler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "SuperscalarScheduler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


SuperscalarScheduler::SuperscalarScheduler(
    Logger& logger,
    WallDuration flush_threshold,
    std::vector<ExecutionResource*> resources
)
    : m_logger(logger)
    , m_flush_threshold(flush_threshold)
    , m_resources(std::move(resources))
{
    clear();
}


void SuperscalarScheduler::clear() noexcept{
//    SpinLockGuard lg(m_lock);
    m_logger.log("Clearing schedule...");
    WallClock now = current_time();
    m_local_start = now;
    m_local_last_activity = now;
    m_device_issue_time = now;
    m_device_sent_time = now;
    m_max_free_time = now;
    m_state_changes.clear();
    for (ExecutionResource* resource : m_resources){
        resource->m_is_busy = false;
        resource->m_busy_time = now;
        resource->m_done_time = now;
        resource->m_free_time = now;
//        cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//             << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//             << ", free_time = " << std::chrono::duration_cast<Milliseconds>((resource->m_free_time - m_local_start)).count()
//             << endl;
    }
    m_pending_clear.store(false, std::memory_order_release);
}

bool SuperscalarScheduler::iterate_schedule(const Cancellable* cancellable){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    if (m_state_changes.empty()){
//        cout << "State is empty." << endl;
        m_device_sent_time = m_device_issue_time;
        return false;
    }

    auto iter = m_state_changes.begin();

    WallClock next_state_change;
    if (m_device_sent_time < *iter){
        next_state_change = *iter;
    }else{
        auto next = iter;
        ++next;
        next_state_change = next == m_state_changes.end()
            ? m_device_issue_time
            : *next;
    }

    //  Things get complicated if we overshoot the issue time.
    if (next_state_change > m_device_issue_time){
//            m_logger.log("SuperscalarScheduler: Overshoot due to unfinished early return.", COLOR_RED);
        next_state_change = m_device_issue_time;
    }

    WallDuration duration = next_state_change - m_device_sent_time;
    if (duration == WallDuration::zero()){
//            m_logger.log("SuperscalarScheduler: Scheduler is stalled.", COLOR_RED);
        return false;
    }

    //  Compute the resource state at this timestamp.
    for (ExecutionResource* resource : m_resources){
        resource->m_is_busy = resource->m_busy_time <= m_device_sent_time && m_device_sent_time < resource->m_done_time;
    }

    m_device_sent_time = next_state_change;
    if (next_state_change >= *iter){
        m_state_changes.erase(iter);
    }

    this->push_state(cancellable, duration);

//    SpinLockGuard lg(m_lock);

    WallClock now = current_time();
    m_local_last_activity = now;

#if 1
    //  If it's been long enough since the issue, we can assume a gap and
    //  fast forward whatever is scheduled into the future.
    if (m_device_sent_time < m_device_issue_time){
        WallDuration time_since_last_activity = now - m_local_last_activity;
        if (time_since_last_activity > m_flush_threshold){
            m_logger.log("SuperscalarScheduler: A dangling early-return issue has gapped.", COLOR_RED);
            m_device_issue_time += time_since_last_activity;

            //  Since we're gapping, we might as well gap a bit more to we don't
            //  re-enter here so quickly.
            m_device_issue_time += m_flush_threshold;

            m_max_free_time = std::max(m_max_free_time, m_device_issue_time);
        }
    }
#endif

    return true;
}
void SuperscalarScheduler::process_schedule(const Cancellable* cancellable){
    //  Any exception is either an unrecoverable error, or an async-cancel.
    //  Both cases should clear the scheduler.
    try{
        while (iterate_schedule(cancellable));
    }catch (...){
        clear();
        throw;
    }
}

void SuperscalarScheduler::issue_wait_for_all(const Cancellable* cancellable){
    if (m_pending_clear.load(std::memory_order_acquire)){
        clear();
        return;
    }
//    cout << "issue_wait_for_all(): " << m_state_changes.size() << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << ", max_free_time = " << std::chrono::duration_cast<Milliseconds>((m_max_free_time - m_local_start)).count()
//         << endl;
    m_device_issue_time = std::max(m_device_issue_time, m_max_free_time);
    m_max_free_time = m_device_issue_time;
    process_schedule(cancellable);
}
void SuperscalarScheduler::issue_nop(const Cancellable* cancellable, WallDuration delay){
    if (delay <= WallDuration::zero()){
        return;
    }
    if (m_pending_clear.load(std::memory_order_acquire)){
        clear();
    }
//    cout << "issue_nop(): " << m_state_changes.size() << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << ", max_free_time = " << std::chrono::duration_cast<Milliseconds>((m_max_free_time - m_local_start)).count()
//         << endl;
    WallClock next_issue_time = m_device_issue_time + delay;
    m_state_changes.insert(next_issue_time);
    m_device_issue_time = next_issue_time;
    m_max_free_time = std::max(m_max_free_time, m_device_issue_time);
    m_local_last_activity = current_time();
    process_schedule(cancellable);
}
void SuperscalarScheduler::wait_for_resource(const Cancellable* cancellable, ExecutionResource& resource){
    if (m_pending_clear.load(std::memory_order_acquire)){
        clear();
        return;
    }
//    cout << "wait_for_resource()" << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << ", free_time = " << std::chrono::duration_cast<Milliseconds>((resource.m_free_time - m_local_start)).count()
//         << endl;
    //  Resource is not ready yet. Stall until it is.
    if (resource.m_free_time > m_device_sent_time){
//        cout << "stall = " << std::chrono::duration_cast<Milliseconds>(resource.m_free_time - m_device_sent_time).count() << endl;
        m_device_issue_time = resource.m_free_time;
        m_local_last_activity = current_time();
    }
    process_schedule(cancellable);
}
void SuperscalarScheduler::issue_to_resource(
    const Cancellable* cancellable, ExecutionResource& resource,
    WallDuration delay, WallDuration hold, WallDuration cooldown
){
    if (m_pending_clear.load(std::memory_order_acquire)){
        clear();
    }

    //  Resource is busy.
    if (m_device_sent_time < resource.m_free_time){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Attempted to issue resource that isn't ready."
        );
    }

    delay    = std::max(delay, WallDuration::zero());
    hold     = std::max(hold, WallDuration::zero());
    cooldown = std::max(cooldown, WallDuration::zero());

//    cout << "issue_to_resource(): delay = " << std::chrono::duration_cast<Milliseconds>(delay).count()
//         << ", hold = " << std::chrono::duration_cast<Milliseconds>(hold).count()
//         << ", cooldown = " << std::chrono::duration_cast<Milliseconds>(cooldown).count()
//         << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << endl;

    WallClock release_time = m_device_issue_time + hold;
    WallClock free_time = release_time + cooldown;

    m_state_changes.insert(m_device_issue_time);
    m_state_changes.insert(release_time);

    resource.m_busy_time = m_device_issue_time;
    resource.m_done_time = release_time;
    resource.m_free_time = free_time;

    m_device_issue_time += delay;
    m_max_free_time = std::max(m_max_free_time, free_time);
    m_max_free_time = std::max(m_max_free_time, m_device_issue_time);
    m_local_last_activity = current_time();

    process_schedule(cancellable);
}






}

