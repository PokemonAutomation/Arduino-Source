/*  OS Sleep (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <condition_variable>
#include <Windows.h>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "SystemSleep.h"

namespace PokemonAutomation{


class WindowsSleepController : public SystemSleepController{
public:
    virtual ~WindowsSleepController(){
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_stopping = true;
            m_cv.notify_all();
        }
        m_thread.join();
        if (m_state.load(std::memory_order_relaxed) != SleepSuppress::NONE){
            global_logger_tagged().log("Destroying WindowsSleepController with active requests...", COLOR_RED);
        }
    }
    WindowsSleepController()
        : m_screen_on_requests(0)
        , m_no_sleep_requests(0)
        , m_stopping(false)
        , m_thread(&WindowsSleepController::thread_loop, this)
    {}

    virtual void push_screen_on() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_screen_on_requests++;
        m_cv.notify_all();
    }
    virtual void pop_screen_on() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_screen_on_requests--;
        m_cv.notify_all();
    }
    virtual void push_no_sleep() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_no_sleep_requests++;
        m_cv.notify_all();
    }
    virtual void pop_no_sleep() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_no_sleep_requests--;
        m_cv.notify_all();
    }

private:
    void thread_loop(){
        //  SetThreadExecutionState(ES_CONTINUOUS) only lasts as long as the
        //  thread is alive. So we use our own thread.
        while (true){
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping){
                return;
            }

            SleepSuppress before_state = m_state.load(std::memory_order_relaxed);
            SleepSuppress after_state = SleepSuppress::NONE;

            EXECUTION_STATE flags = ES_CONTINUOUS;
            if (m_no_sleep_requests > 0){
                flags |= ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED;
                after_state = SleepSuppress::NO_SLEEP;
            }
            if (m_screen_on_requests > 0){
                flags |= ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED;
                after_state = SleepSuppress::SCREEN_ON;
            }
            EXECUTION_STATE ret = SetThreadExecutionState(flags);
            if (ret == 0){
                global_logger_tagged().log("Unable to set system sleep state.", COLOR_RED);
            }else{
                switch (after_state){
                case SleepSuppress::NONE:
                    global_logger_tagged().log("Setting sleep state to: None", COLOR_BLUE);
                    break;
                case SleepSuppress::NO_SLEEP:
                    global_logger_tagged().log("Setting sleep state to: No Sleep", COLOR_BLUE);
                    break;
                case SleepSuppress::SCREEN_ON:
                    global_logger_tagged().log("Setting sleep state to: Screen On", COLOR_BLUE);
                    break;
                }

                m_state.store(after_state, std::memory_order_release);

                if (before_state != after_state){
                    notify_listeners(after_state);
                }
            }

            m_cv.wait(lg);
        }
    }

private:
    size_t m_screen_on_requests = 0;
    size_t m_no_sleep_requests = 0;

    bool m_stopping;
    std::condition_variable m_cv;
    std::thread m_thread;
};


SystemSleepController& SystemSleepController::instance(){
    static WindowsSleepController controller;
    return controller;
}






}
