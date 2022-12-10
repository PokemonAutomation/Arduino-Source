/*  Scheduled Task Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Run tasks at their scheduled times.
 *
 */

#ifndef PokemonAutomation_ScheduledTaskRunner_H
#define PokemonAutomation_ScheduledTaskRunner_H

#include <map>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"

namespace PokemonAutomation{


class ScheduledTaskRunner final : public Cancellable{
public:
    ~ScheduledTaskRunner();
    ScheduledTaskRunner(AsyncDispatcher& dispatcher);

    size_t size() const;

    void add_event(WallClock time, std::function<void()> callback);
    void add_event(std::chrono::milliseconds time_from_now, std::function<void()> callback);

    virtual bool cancel(std::exception_ptr exception) noexcept override;

private:
    void thread_loop();

private:
    mutable std::mutex m_lock;
    std::condition_variable m_cv;

    std::multimap<WallClock, std::function<void()>> m_schedule;

    std::unique_ptr<AsyncTask> m_runner;
};



}
#endif
