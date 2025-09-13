/*  Superscalar Scheduler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SuperscalarScheduler_H
#define PokemonAutomation_Controllers_SuperscalarScheduler_H

#include <memory>
#include <set>
#include <map>
#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AbstractLogger.h"
//#include "Common/Cpp/CancellableScope.h"

namespace PokemonAutomation{

class SuperscalarScheduler;




class SchedulerResource{
public:
    const size_t id;

    SchedulerResource(size_t id)
        : id(id)
    {}

    virtual ~SchedulerResource() = default;
};





class SuperscalarScheduler{
public:
    using State = std::vector<std::shared_ptr<const SchedulerResource>>;
    struct ScheduleEntry{
        WallDuration duration;
        State state;
    };
    using Schedule = std::vector<ScheduleEntry>;

public:
    SuperscalarScheduler(Logger& logger, WallDuration flush_threshold);

    void clear_on_next(){
        m_pending_clear = true;
    }


public:
    //  These are the standard "issue" commands.
    //
    //  These functions are non-blocking and will append any new schedule
    //  entries to "schedule".
    //
    //  These are not thread-safe with each other.
    //

    WallClock busy_until(size_t resource_id) const{
        auto iter = m_live_commands.find(resource_id);
        return iter != m_live_commands.end()
            ? iter->second.free_time
            : WallClock::min();
    }

    //  Wait until the pipeline has completely cleared and all resources have
    //  returned to the ready state.
    void issue_wait_for_all(Schedule& schedule);

    //  Issue a do-nothing command for the specified delay.
    //  This will advance the issue timestamp.
    void issue_nop(Schedule& schedule, WallDuration delay);

    //  Wait until the specified resource is ready to be used.
    //  This will advance the issue timestamp until the resource is ready.
    void issue_wait_for_resource(Schedule& schedule, size_t resource_id);

    //  Issue a resource with the specified timing parameters.
    void issue_to_resource(
        Schedule& schedule,
        std::shared_ptr<const SchedulerResource> resource,
        WallDuration delay, WallDuration hold, WallDuration cooldown
    );


private:
    void clear() noexcept;
    State current_live_commands();
    void clear_finished_commands();
    bool iterate_schedule(Schedule& schedule);
    void process_schedule(Schedule& schedule);


private:
    Logger& m_logger;

    //  We are allowed to gap the timeline if nothing has been issued in this
    //  much time.
    const WallDuration m_flush_threshold;

    bool m_pending_clear;

    //  The construction time of this object. This is only used for debugging
    //  purposes since it lets you print wall times relative to this.
    WallClock m_local_start;

    //  Wall clock of the last time "m_device_issue_time" was last updated.
    //  This is used to decide when to gap the timeline.
    WallClock m_local_last_activity;

    //  The current timestamp of what has been issued to the scheduler.
    WallClock m_device_issue_time;

    //  The current timestamp of what has been sent to the device.
    WallClock m_device_sent_time;

    //  Maximum of: m_live_commands[]->second.free_time
    WallClock m_max_free_time;

    //  A set of all the scheduled state changes that will happen. Between
    //  timestamps in this set, the state is constant.
    std::set<WallClock> m_state_changes;

    struct Command{
        std::shared_ptr<const SchedulerResource> command;
        WallClock busy_time;    //  Timestamp of when resource will be become busy.
        WallClock done_time;    //  Timestamp of when resource will be done being busy.
        WallClock free_time;    //  Timestamp of when resource can be used again.
    };
    std::map<size_t, Command> m_live_commands;
};





}
#endif
