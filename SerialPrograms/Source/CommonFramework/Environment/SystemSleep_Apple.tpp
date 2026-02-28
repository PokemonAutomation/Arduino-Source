/*  OS Sleep (Apple)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include "CommonFramework/Logging/Logger.h"
#include "SystemSleep.h"

namespace PokemonAutomation{



class AppleSleepController final : public SystemSleepController{
public:
    virtual ~AppleSleepController(){
        stop();
    }
    virtual void stop() noexcept override{
        std::lock_guard<Mutex> lg(m_lock);
        update_state(SleepSuppress::NONE);
    }
    virtual void push_screen_on() override{
        std::lock_guard<Mutex> lg(m_lock);
        update_state(SleepSuppress::SCREEN_ON);
    }
    virtual void pop_screen_on() override{
        std::lock_guard<Mutex> lg(m_lock);
        update_state(SleepSuppress::NONE);
    }
    virtual void push_no_sleep() override{
        std::lock_guard<Mutex> lg(m_lock);
        update_state(SleepSuppress::NO_SLEEP);
    }
    virtual void pop_no_sleep() override{
        std::lock_guard<Mutex> lg(m_lock);
        update_state(SleepSuppress::NONE);
    }


private:
    void update_state(SleepSuppress state); // call after holding the lock
    bool disable_sleep(CFStringRef prevention_type); // call after holding the lock
    bool enable_sleep(); // call after holding the lock

    IOPMAssertionID m_session_id = kIOPMNullAssertionID;
};


bool AppleSleepController::disable_sleep(CFStringRef prevention_type){
    if (m_session_id != kIOPMNullAssertionID){ // already disabled
        return true;
    }
    auto ret = IOPMAssertionCreateWithDescription(
        prevention_type,
        CFSTR("SerialPrograms"),
        CFSTR("SerialPrograms is running"),
        nullptr,
        nullptr,
        0,
        nullptr, &m_session_id
    );
    if (ret != kIOReturnSuccess){
        m_session_id = kIOPMNullAssertionID;
        global_logger_tagged().log("Unable to disaable sleep. Error code " + std::to_string(ret), COLOR_RED);
        return false;
    }
    return true;
}

bool AppleSleepController::enable_sleep(){
    if (m_session_id != kIOPMNullAssertionID){
        auto ret = IOPMAssertionRelease(m_session_id);
        if (ret != kIOReturnSuccess){
            global_logger_tagged().log("Unable to enable sleep. Error code " + std::to_string(ret), COLOR_RED);
            return false;
        }
        m_session_id = kIOPMNullAssertionID;
    }
    return true;
}



void AppleSleepController::update_state(SleepSuppress state){
    SleepSuppress before_state = m_state.load(std::memory_order_relaxed);

    // kIOPMAssertPreventUserIdleSystemSleep:
    //    Prevents the system from sleeping automatically due to a lack of user activity.
    // kIOPMAssertPreventUserIdleDisplaySleep:
    //    Prevents the display from dimming automatically.
    // kIOPMAssertionTypeNoDisplaySleep prevents display sleep - Deprecated in 10.7
    // kIOPMAssertionTypeNoIdleSleep prevents idle sleep - Deprecated in 10.7

    switch (state){
    case SleepSuppress::NONE:
        if (enable_sleep()){
            global_logger_tagged().log("Enabled display sleep and OS sleep.", COLOR_BLUE);
        }
        break;
    case SleepSuppress::NO_SLEEP:
        global_logger_tagged().log("Disabling OS sleep...", COLOR_BLUE);
        if (disable_sleep(kIOPMAssertPreventUserIdleSystemSleep)){
            global_logger_tagged().log("Disabled OS sleep.", COLOR_BLUE);
        }
        break;
    case SleepSuppress::SCREEN_ON:
        global_logger_tagged().log("Disabling display sleep and OS sleep...", COLOR_BLUE);
        if (disable_sleep(kIOPMAssertPreventUserIdleDisplaySleep)){
            global_logger_tagged().log("Disabled display sleep and OS sleep.", COLOR_BLUE);
        }
        break;
    }

    m_state.store(state, std::memory_order_release);

    if (before_state != state){
        notify_listeners(state);
    }
}


SystemSleepController& SystemSleepController::instance(){
    static AppleSleepController controller;
    return controller;
}






}
