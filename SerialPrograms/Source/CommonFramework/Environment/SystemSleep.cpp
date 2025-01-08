/*  OS Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "SystemSleep.h"

#if 0
#elif _WIN32
#include "SystemSleep_Windows.tpp"
#elif __APPLE__
#include "SystemSleep_Apple.tpp"
#else
namespace PokemonAutomation{
    SystemSleepController& SystemSleepController::instance(){
        static SystemSleepController controller;
        return controller;
    }
}
#endif

namespace PokemonAutomation{



SystemSleepController::SystemSleepController()
    : m_state(SleepSuppress::NONE)
{}

void SystemSleepController::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void SystemSleepController::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}
void SystemSleepController::notify_listeners(SleepSuppress state){
    for (Listener* listener : m_listeners){
        listener->sleep_suppress_state_changed(state);
    }
}

SleepSuppress SystemSleepController::current_state() const{
    return m_state.load(std::memory_order_relaxed);
}







void SleepSuppressScope::clear(){
    switch (m_mode){
    case SleepSuppress::NONE:
        break;
    case SleepSuppress::NO_SLEEP:
        SystemSleepController::instance().pop_no_sleep();
        break;
    case SleepSuppress::SCREEN_ON:
        SystemSleepController::instance().pop_screen_on();
        break;
    }
        m_mode = SleepSuppress::NONE;
}
void SleepSuppressScope::operator=(SleepSuppress mode){
    clear();
    set(mode);
}
void SleepSuppressScope::set(SleepSuppress mode){
    switch (mode){
    case SleepSuppress::NONE:
        break;
    case SleepSuppress::NO_SLEEP:
        SystemSleepController::instance().push_no_sleep();
        break;
    case SleepSuppress::SCREEN_ON:
        SystemSleepController::instance().push_screen_on();
        break;
    }
    m_mode = mode;
}




}

