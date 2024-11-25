/*  OS Sleep (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <Windows.h>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "SystemSleep.h"

namespace PokemonAutomation{


class WindowsSleepController : public SystemSleepController{
public:
    virtual ~WindowsSleepController(){
        std::lock_guard<std::mutex> lg(m_lock);
        m_screen_on_requests = 0;
        m_no_sleep_requests = 0;
        update_state();
    }
    virtual void push_screen_on() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_screen_on_requests++;
        update_state();
    }
    virtual void pop_screen_on() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_screen_on_requests--;
        update_state();
    }
    virtual void push_no_sleep() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_no_sleep_requests++;
        update_state();
    }
    virtual void pop_no_sleep() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_no_sleep_requests--;
        update_state();
    }

private:
    void update_state(){
        //  SetThreadExecutionState(ES_CONTINUOUS) only lasts as long as the
        //  thread is alive. So we redispatch to the main thread.
        queue_on_main_thread([
            screen_on_requests = m_screen_on_requests,
            no_sleep_requests = m_no_sleep_requests
        ]{
            EXECUTION_STATE flags = ES_CONTINUOUS;
            if (screen_on_requests > 0){
                flags |= ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED;
            }
            if (no_sleep_requests > 0){
                flags |= ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED;
            }
            EXECUTION_STATE ret = SetThreadExecutionState(flags);
            if (ret == 0){
                global_logger_tagged().log("Unable to set system sleep state.", COLOR_RED);
            }else if (screen_on_requests > 0){
                global_logger_tagged().log("Setting sleep state to: Screen On", COLOR_BLUE);
            }else if (no_sleep_requests > 0){
                global_logger_tagged().log("Setting sleep state to: No Sleep", COLOR_BLUE);
            }else{
                global_logger_tagged().log("Setting sleep state to: None", COLOR_BLUE);
            }
        });
    }

private:
    std::mutex m_lock;
    size_t m_screen_on_requests = 0;
    size_t m_no_sleep_requests = 0;
};


SystemSleepController& SystemSleepController::instance(){
    static WindowsSleepController controller;
    return controller;
}






}
