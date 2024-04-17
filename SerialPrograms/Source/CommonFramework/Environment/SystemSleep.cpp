/*  OS Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "SystemSleep.h"

#include <iostream>

namespace PokemonAutomation{

#if defined(__APPLE__)

// Code from https://stackoverflow.com/questions/5596319/how-to-programmatically-prevent-a-mac-from-going-to-sleep/8461182#8461182

SystemSleepController::SystemSleepController()
 : m_prevention_succeeded(kIOReturnError), m_session_id(0) {}

SystemSleepController::~SystemSleepController(){
    enable_sleep();
}

bool SystemSleepController::prevent_sleep(bool prevent){
    if (prevent){
        return disable_sleep();
    }else{
        return enable_sleep();
    }
}

bool SystemSleepController::disable_sleep(){
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

bool SystemSleepController::enable_sleep(){
    if (m_prevention_succeeded == kIOReturnSuccess){
        IOPMAssertionRelease(m_session_id);
        m_session_id = 0;
        m_prevention_succeeded = kIOReturnError;
        std::cout << "Enabled display sleep and OS sleep." << std::endl;
    }
    return true;
}

#endif // defined(__APPLE__)

}

