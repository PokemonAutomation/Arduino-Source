/*  OS Sleep (Apple)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <mutex>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include "CommonFramework/Logging/Logger.h"
#include "SystemSleep.h"

namespace PokemonAutomation{



class AppleSleepController : public SystemSleepController{
public:
    virtual ~AppleSleepController(){
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
    //  Disable/Enable screen saver and OS sleep.
    //  Return whether the setting is successful.
    bool prevent_sleep(bool prevent);
    void update_state();

    bool disable_sleep();
    bool enable_sleep();

    IOReturn m_prevention_succeeded = kIOReturnError;
    IOPMAssertionID m_session_id = 0;

    size_t m_screen_on_requests = 0;
    size_t m_no_sleep_requests = 0;
};


// Code from https://stackoverflow.com/questions/5596319/how-to-programmatically-prevent-a-mac-from-going-to-sleep/8461182#8461182


bool AppleSleepController::prevent_sleep(bool prevent){
    if (prevent){
        return disable_sleep();
    }else{
        return enable_sleep();
    }
}

bool AppleSleepController::disable_sleep(){
    if (m_prevention_succeeded == kIOReturnSuccess){
        return true;
    }
    std::cout << "Disabling display sleep and OS sleep" << std::endl;
    // kIOPMAssertionTypeNoDisplaySleep prevents display sleep,
    // kIOPMAssertionTypeNoIdleSleep prevents idle sleep

    // NOTE: IOPMAssertionCreateWithName limits the string to 128 characters.
    CFStringRef reasonForActivity = (CFStringRef) __builtin___CFStringMakeConstantString("SerialPrograms is running");

    m_prevention_succeeded = kIOReturnError;
    m_session_id = 0;
    m_prevention_succeeded = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                         kIOPMAssertionLevelOn, reasonForActivity, &m_session_id);
    if (m_prevention_succeeded != kIOReturnSuccess){
        m_session_id = 0;
        std::cerr << "Cannot disable sleep. Error code " << m_prevention_succeeded << std::endl;
        return false;
    }

    std::cout << "Disabled display sleep and OS sleep" << std::endl;
    return true;
}

bool AppleSleepController::enable_sleep(){
    if (m_prevention_succeeded == kIOReturnSuccess){
        IOPMAssertionRelease(m_session_id);
        m_session_id = 0;
        m_prevention_succeeded = kIOReturnError;
        std::cout << "Enabled display sleep and OS sleep." << std::endl;
    }
    return true;
}



void AppleSleepController::update_state(){
    //  Must call under lock.

    SleepSuppress before_state = m_state.load(std::memory_order_relaxed);
    SleepSuppress after_state = SleepSuppress::NONE;

    //  TODO: Distiguish these two.
    bool enabled = m_screen_on_requests > 0 || m_no_sleep_requests > 0;
    prevent_sleep(enabled);

    after_state = enabled
        ? SleepSuppress::SCREEN_ON
        : SleepSuppress::NONE,

    m_state.store(after_state, std::memory_order_release);

    if (before_state != after_state){
        notify_listeners(after_state);
    }
}


SystemSleepController& SystemSleepController::instance(){
    static AppleSleepController controller;
    return controller;
}






}
