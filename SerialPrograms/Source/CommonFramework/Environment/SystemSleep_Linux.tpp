/*  OS Sleep (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sdbus-c++/sdbus-c++.h>
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "SystemSleep.h"

namespace PokemonAutomation{


class LinuxSleepController final : public SystemSleepController{
public:
    LinuxSleepController()
        : m_screen_on_requests(0)
        , m_no_sleep_requests(0)
        , m_stopping(false)
        , m_thread(GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
            [this]{ thread_loop(); })
        )
    {}
    virtual ~LinuxSleepController(){
        stop();
    }
    virtual void stop() noexcept override{
        if (!m_thread){
            return;
        }
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_stopping = true;
        }
        m_cv.notify_all();
        m_thread.wait_and_ignore_exceptions();
        m_inhibit_fd.reset();
        if (m_state.load(std::memory_order_relaxed) != SleepSuppress::NONE){
            try{
                global_logger_tagged().log(
                    "Destroying LinuxSleepController with active requests...",
                    COLOR_RED
                );
            }catch (...){}
        }
    }

    virtual void push_screen_on() override{
        std::lock_guard<Mutex> lg(m_lock);
        m_screen_on_requests++;
        m_cv.notify_all();
    }
    virtual void pop_screen_on() override{
        std::lock_guard<Mutex> lg(m_lock);
        m_screen_on_requests--;
        m_cv.notify_all();
    }
    virtual void push_no_sleep() override{
        std::lock_guard<Mutex> lg(m_lock);
        m_no_sleep_requests++;
        m_cv.notify_all();
    }
    virtual void pop_no_sleep() override{
        std::lock_guard<Mutex> lg(m_lock);
        m_no_sleep_requests--;
        m_cv.notify_all();
    }

private:
    void thread_loop()
    {
        auto connection = sdbus::createSystemBusConnection();
        auto proxy = sdbus::createProxy(
            *connection,
            sdbus::BusName{"org.freedesktop.login1"},
            sdbus::ObjectPath{"/org/freedesktop/login1"}
        );

        while (true)
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping)
            {
                return;
            }

            SleepSuppress before_state = m_state.load(std::memory_order_relaxed);
            SleepSuppress after_state = SleepSuppress::NONE;

            bool need_inhibit = false;
            std::string what;
            if (m_no_sleep_requests > 0)
            {
                need_inhibit = true;
                what = "sleep";
                after_state = SleepSuppress::NO_SLEEP;
            }
            else if (m_screen_on_requests > 0)
            {
                need_inhibit = true;
                what = "idle";
                after_state = SleepSuppress::SCREEN_ON;
            }
            if (need_inhibit && !m_inhibit_fd)
            {
                try
                {
                    sdbus::UnixFd fd;
                    proxy->callMethod("Inhibit")
                        .onInterface("org.freedesktop.login1.Manager")
                        .withArguments(what, "PokemonAutomation", "Keeping system awake", "block")
                        .storeResultsTo(fd);
                    m_inhibit_fd = std::make_unique<sdbus::UnixFd>(std::move(fd));

                    global_logger_tagged().log("Acquired sleep inhibitor", COLOR_BLUE);
                }

                catch (const sdbus::Error &e)
                {
                    global_logger_tagged().log(std::string("DBus error: ") + e.what(), COLOR_RED);
                }
            }
            else if (!need_inhibit && m_inhibit_fd)
            {
                m_inhibit_fd.reset();
                global_logger_tagged().log("Released sleep inhibitor", COLOR_BLUE);
            }

            if (before_state != after_state)
            {
                m_state.store(after_state, std::memory_order_release);
                notify_listeners(after_state);
            }

            m_cv.wait(lg);
        }
    }

private:
    size_t m_screen_on_requests = 0;
    size_t m_no_sleep_requests = 0;

    bool m_stopping;
    ConditionVariable m_cv;
    AsyncTask m_thread;

    std::unique_ptr<sdbus::UnixFd> m_inhibit_fd;
};


SystemSleepController& SystemSleepController::instance(){
    static LinuxSleepController controller;
    return controller;
}






}
