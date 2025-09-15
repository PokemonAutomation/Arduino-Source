/*  Superscalar Scheduler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "SuperscalarScheduler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


SuperscalarScheduler::SuperscalarScheduler(
    Logger& logger,
    WallDuration flush_threshold
)
    : m_logger(logger)
    , m_flush_threshold(flush_threshold)
{
    clear();
}


void SuperscalarScheduler::clear() noexcept{
//    SpinLockGuard lg(m_lock);
//    m_logger.log("Clearing schedule...");
    WallClock now = current_time();
    m_local_start = now;
    m_local_last_activity = now;
    m_device_issue_time = now;
    m_device_sent_time = now;
    m_max_free_time = now;
    m_state_changes.clear();
    m_live_commands.clear();
    m_pending_clear = false;
}

SuperscalarScheduler::State SuperscalarScheduler::current_live_commands(){
    WallClock device_sent_time = m_device_sent_time;
    State ret;
//    cout << "device_sent_time = " << std::chrono::duration_cast<Milliseconds>(device_sent_time - m_local_start).count() << endl;
    for (auto& item : m_live_commands){
//        cout << "busy = " << std::chrono::duration_cast<Milliseconds>(item.second.busy_time - m_local_start).count()
//             << ", done = " << std::chrono::duration_cast<Milliseconds>(item.second.done_time - m_local_start).count() << endl;
        if (item.second.busy_time <= device_sent_time && device_sent_time < item.second.done_time){
            ret.emplace_back(item.second.command);
        }
    }
    return ret;
}
void SuperscalarScheduler::clear_finished_commands(){
    WallClock device_sent_time = m_device_sent_time;
    for (auto iter = m_live_commands.begin(); iter != m_live_commands.end();){
//        cout << "device_sent_time = " << device_sent_time << ", free_time = " << iter->second.free_time << endl;
        if (device_sent_time >= iter->second.free_time){
            iter = m_live_commands.erase(iter);
        }else{
            ++iter;
        }
    }
}
bool SuperscalarScheduler::iterate_schedule(Schedule& schedule){
//    cout << "----------------------------> " << m_state_changes.size() << endl;
//    cout << "m_device_sent_time = " << std::chrono::duration_cast<Milliseconds>(m_device_sent_time - m_local_start) << endl;

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
//        m_logger.log("SuperscalarScheduler: Overshoot due to unfinished early return.", COLOR_RED);
        next_state_change = m_device_issue_time;
    }

    WallDuration duration = next_state_change - m_device_sent_time;
    if (duration == WallDuration::zero()){
        //  This happens when the command has a delay of zero. A delay of zero
        //  is almost always immediately followed by another command that is
        //  intended to execute simultaneously. Thus we do not attempt to
        //  execute the schedule any further.
//        m_logger.log("SuperscalarScheduler: Scheduler is stalled.", COLOR_RED);
        return false;
    }

    //  Compute the resource state at this timestamp.
    std::vector<std::shared_ptr<const SchedulerResource>> state = current_live_commands();
    clear_finished_commands();

    m_device_sent_time = next_state_change;
    if (next_state_change > *iter){
        m_state_changes.erase(iter);
    }

    ScheduleEntry& entry = schedule.emplace_back();
    entry.duration = duration;
    entry.state = std::move(state);

//    SpinLockGuard lg(m_lock);

    WallClock now = current_time();
    m_local_last_activity = now;

    //  If we are not dangling anything, we can return now.
    if (m_device_sent_time >= m_device_issue_time){
        return true;
    }

#if 1
    //
    //  We are currently dangling a command. We don't know whether the current
    //  ongoing commands will run out, or if something new will join them in the
    //  near future.
    //
    //  If it's been long enough since the last issue, we can assume that
    //  nothing will join and thus can gap. (We do require that dangling
    //  commands be resolved promptly or they may run out.)
    //
    //  Thus we can fast forward whatever is scheduled into the future.
    //
    WallDuration time_since_last_activity = now - m_local_last_activity;
    if (time_since_last_activity > m_flush_threshold){
        m_logger.log("SuperscalarScheduler: A dangling early-return issue has gapped.", COLOR_RED);
        m_device_issue_time += time_since_last_activity;

        //  Since we're gapping, we might as well gap a bit more to we don't
        //  re-enter here so quickly.
        m_device_issue_time += m_flush_threshold;

        m_max_free_time = std::max(m_max_free_time, m_device_issue_time);
    }

#endif

    return true;
}
void SuperscalarScheduler::process_schedule(Schedule& schedule){
    //  Any exception is either an unrecoverable error, or an async-cancel.
    //  Both cases should clear the scheduler.
    try{
        while (iterate_schedule(schedule));
    }catch (...){
        clear();
        throw;
    }
}

void SuperscalarScheduler::issue_wait_for_all(Schedule& schedule){
    if (m_pending_clear){
        clear();
        return;
    }
//    m_logger.log("issue_wait_for_all(): states = " + std::to_string(m_state_changes.size()), COLOR_DARKGREEN);
//    cout << "issue_wait_for_all(): " << m_state_changes.size() << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << ", max_free_time = " << std::chrono::duration_cast<Milliseconds>((m_max_free_time - m_local_start)).count()
//         << endl;
    m_device_issue_time = std::max(m_device_issue_time, m_max_free_time);
    m_max_free_time = m_device_issue_time;
    process_schedule(schedule);
}
void SuperscalarScheduler::issue_nop(Schedule& schedule, WallDuration delay){
    if (delay <= WallDuration::zero()){
        return;
    }
    if (m_pending_clear){
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
    process_schedule(schedule);
}
void SuperscalarScheduler::issue_wait_for_resource(Schedule& schedule, size_t resource_id){
    if (m_pending_clear){
        clear();
        return;
    }
//    cout << "wait_for_resource()" << endl;
//    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
//         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
//         << ", free_time = " << std::chrono::duration_cast<Milliseconds>((resource.m_free_time - m_local_start)).count()
//         << endl;

    //  Resource is not ready yet. Stall until it is.
    auto iter = m_live_commands.find(resource_id);
    if (iter != m_live_commands.end() && m_device_sent_time < iter->second.free_time){
        m_device_issue_time = iter->second.free_time;
        m_local_last_activity = current_time();
    }

    process_schedule(schedule);
}
void SuperscalarScheduler::issue_to_resource(
    Schedule& schedule,
    std::shared_ptr<const SchedulerResource> resource,
    WallDuration delay, WallDuration hold, WallDuration cooldown
){
    if (m_pending_clear){
        clear();
    }

    //  Resource is busy. Stall until it is free.
    auto ret = m_live_commands.try_emplace(resource->id);
    if (!ret.second){
//        cout << m_device_sent_time << " : " << ret.first->second.free_time << endl;
        m_device_issue_time = std::max(m_device_issue_time, ret.first->second.free_time);
        process_schedule(schedule);
    }
    Command& command = ret.first->second;

    delay    = std::max(delay, WallDuration::zero());
    hold     = std::max(hold, WallDuration::zero());
    cooldown = std::max(cooldown, WallDuration::zero());

#if 0
    cout << "issue_to_resource(): delay = " << std::chrono::duration_cast<Milliseconds>(delay).count()
         << ", hold = " << std::chrono::duration_cast<Milliseconds>(hold).count()
         << ", cooldown = " << std::chrono::duration_cast<Milliseconds>(cooldown).count()
         << endl;
    cout << "issue_time = " << std::chrono::duration_cast<Milliseconds>((m_device_issue_time - m_local_start)).count()
         << ", sent_time = " << std::chrono::duration_cast<Milliseconds>((m_device_sent_time - m_local_start)).count()
         << endl;
#endif

    WallClock release_time = m_device_issue_time + hold;
    WallClock free_time = release_time + cooldown;

    m_state_changes.insert(m_device_issue_time);
    m_state_changes.insert(release_time);

    command.command = std::move(resource);
    command.busy_time = m_device_issue_time;
    command.done_time = release_time;
    command.free_time = free_time;

    m_device_issue_time += delay;
    m_max_free_time = std::max(m_max_free_time, free_time);
    m_max_free_time = std::max(m_max_free_time, m_device_issue_time);
    m_local_last_activity = current_time();

    process_schedule(schedule);
}






}

