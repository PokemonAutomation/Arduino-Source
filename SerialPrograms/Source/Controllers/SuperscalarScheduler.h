/*  Superscalar Scheduler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SuperscalarScheduler_H
#define PokemonAutomation_Controllers_SuperscalarScheduler_H

#include <set>
#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/CancellableScope.h"

namespace PokemonAutomation{

class SuperscalarScheduler;


class ExecutionResource{
public:
    virtual ~ExecutionResource() = default;
    ExecutionResource()
        : m_busy_time(WallClock::min())
        , m_done_time(WallClock::min())
        , m_free_time(WallClock::min())
    {}

    bool is_busy() const{
        return m_is_busy;
    }
    WallClock free_time() const{
        return m_free_time;
    }

private:
    friend class SuperscalarScheduler;
    bool m_is_busy = false;
    WallClock m_busy_time;  //  Timestamp of when resource will be become busy.
    WallClock m_done_time;  //  Timestamp of when resource will be done being busy.
    WallClock m_free_time;  //  Timestamp of when resource can be used again.
};





class SuperscalarScheduler{
public:
    SuperscalarScheduler(
        Logger& logger,
        WallDuration flush_threshold,
        std::vector<ExecutionResource*> resources
    );


public:
    //  This is intended to be run on a different thread from the functions in
    //  the next section. So it is safe to run from anywhere.

    void clear_on_next(){
        m_pending_clear.store(true, std::memory_order_release);
    }


public:
    //  These are the standard "issue" commands.
    //
    //  These functions may or may not block.
    //  These are not thread-safe with each other.
    //

    //  Wait until the pipeline has completely cleared and all resources have
    //  returned to the ready state.
    void issue_wait_for_all(const Cancellable* cancellable);

    //  Issue a do-nothing command for the specified delay.
    //  This will advance the issue timestamp.
    void issue_nop(const Cancellable* cancellable, WallDuration delay);

    //  Wait until the specified resource is ready to be used.
    //  This will advance the issue timestamp until the resource is ready.
    void issue_wait_for_resource(const Cancellable* cancellable, ExecutionResource& resource);

    //  Issue a resource with the specified timing parameters.
    //  The resource must be ready to be used.
    void issue_to_resource(
        const Cancellable* cancellable, ExecutionResource& resource,
        WallDuration delay, WallDuration hold, WallDuration cooldown
    );


protected:
    //
    //  This class will automatically call "push_state()" when a state is ready.
    //  The child class should send/queue this state to the device/Switch.
    //
    //  This function is allowed to block. In other words, it can wait until
    //  there is space in the queue before returning.
    //
    //  This will always be called inside one of the above "issue_XXX()"
    //  functions. Implementations of this method should be aware of this
    //  re-entrancy when this gets called on them with respect to locking.
    //
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) = 0;


private:
    void clear() noexcept;
    bool iterate_schedule(const Cancellable* cancellable);
    void process_schedule(const Cancellable* cancellable);


private:
    Logger& m_logger;

    //  We are allowed to gap the timeline if nothing has been issued in this
    //  much time.
    const WallDuration m_flush_threshold;

    std::atomic<bool> m_pending_clear;

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

    //  Maximum of: m_resources[]->m_free_time
    WallClock m_max_free_time;

    //  A set of all the scheduled state changes that will happen. Between
    //  timestamps in this set, the state is constant.
    std::set<WallClock> m_state_changes;

    std::vector<ExecutionResource*> m_resources;
};





}
#endif
