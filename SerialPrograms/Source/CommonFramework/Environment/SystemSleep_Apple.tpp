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



class SystemSleepController::InternalController{
public:
    InternalController();
    ~InternalController();

    //  Disable/Enable screen saver and OS sleep.
    //  Return whether the setting is successful.
    bool prevent_sleep(bool prevent);

    void update_state();

private:
    bool disable_sleep();
    bool enable_sleep();

    IOReturn m_prevention_succeeded;
    IOPMAssertionID m_session_id;


    std::mutex lock;
    size_t screen_on_requests = 0;
    size_t no_sleep_requests = 0;
};


// Code from https://stackoverflow.com/questions/5596319/how-to-programmatically-prevent-a-mac-from-going-to-sleep/8461182#8461182

SystemSleepController::InternalController::InternalController()
    : m_prevention_succeeded(kIOReturnError), m_session_id(0) {}

SystemSleepController::InternalController::~InternalController(){
    enable_sleep();
}

bool SystemSleepController::InternalController::prevent_sleep(bool prevent){
    if (prevent){
        return disable_sleep();
    }else{
        return enable_sleep();
    }
}

bool SystemSleepController::InternalController::disable_sleep(){
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

bool SystemSleepController::InternalController::enable_sleep(){
    if (m_prevention_succeeded == kIOReturnSuccess){
        IOPMAssertionRelease(m_session_id);
        m_session_id = 0;
        m_prevention_succeeded = kIOReturnError;
        std::cout << "Enabled display sleep and OS sleep." << std::endl;
    }
    return true;
}



void SystemSleepController::InternalController::update_state(){
    //  Must call under lock.

    //  TODO: Distiguish these two.
    prevent_sleep(screen_on_requests > 0 || no_sleep_requests > 0);
}





}
