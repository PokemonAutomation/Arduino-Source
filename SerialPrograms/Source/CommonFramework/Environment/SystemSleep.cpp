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
    SystemSleepController(){}
    ~SystemSleepController(){}
    void push_screen_on(){}
    void pop_screen_on(){}
    void push_no_sleep(){}
    void pop_no_sleep(){}
}
#endif

namespace PokemonAutomation{



SystemSleepController::SystemSleepController()
    : m_data(new InternalController())
{}
SystemSleepController::~SystemSleepController(){
    std::lock_guard<std::mutex> lg(m_data->lock);
    m_data->screen_on_requests = 0;
    m_data->no_sleep_requests = 0;
    m_data->update_state();
}
void SystemSleepController::push_screen_on(){
    std::lock_guard<std::mutex> lg(m_data->lock);
    m_data->screen_on_requests++;
    m_data->update_state();
}
void SystemSleepController::pop_screen_on(){
    std::lock_guard<std::mutex> lg(m_data->lock);
    m_data->screen_on_requests--;
    m_data->update_state();
}
void SystemSleepController::push_no_sleep(){
    std::lock_guard<std::mutex> lg(m_data->lock);
    m_data->no_sleep_requests++;
    m_data->update_state();
}
void SystemSleepController::pop_no_sleep(){
    std::lock_guard<std::mutex> lg(m_data->lock);
    m_data->no_sleep_requests--;
    m_data->update_state();
}



}

