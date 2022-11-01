/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ConfigOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct ConfigOption::Data{
    mutable SpinLock lock;
    bool lock_while_program_is_running;
    std::atomic<ConfigOptionState> visibility;
    std::set<Listener*> listeners;

    Data(bool p_lock_while_program_is_running, ConfigOptionState p_visibility)
        : lock_while_program_is_running(p_lock_while_program_is_running)
        , visibility(p_visibility)
    {}

    bool set_visibility(ConfigOptionState state){
        while (true){
            ConfigOptionState current = visibility.load(std::memory_order_acquire);

            //  Already in that state. No change.
            if (current == state){
                return false;
            }

            //  Attempt to change.
            if (visibility.compare_exchange_weak(current, state)){
                return true;
            }
        }
    }
};



ConfigOption::~ConfigOption() = default;
ConfigOption::ConfigOption(const ConfigOption& x)
    : m_data(CONSTRUCT_TOKEN, x.lock_while_program_is_running(), x.visibility())
{}

ConfigOption::ConfigOption()
    : m_data(CONSTRUCT_TOKEN, true, ConfigOptionState::ENABLED)
{}
ConfigOption::ConfigOption(LockWhileRunning lock_while_program_is_running)
    : m_data(CONSTRUCT_TOKEN, lock_while_program_is_running == LockWhileRunning::LOCKED, ConfigOptionState::ENABLED)
{}
ConfigOption::ConfigOption(ConfigOptionState visibility)
    : m_data(CONSTRUCT_TOKEN, true, visibility)
{}

void ConfigOption::add_listener(Listener& listener){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    data.listeners.insert(&listener);
}
void ConfigOption::remove_listener(Listener& listener){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    data.listeners.erase(&listener);
}
size_t ConfigOption::total_listeners() const{
    const Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    return data.listeners.size();
}





void ConfigOption::load_json(const JsonValue& json){}
JsonValue ConfigOption::to_json() const{
    return JsonValue();
}
bool ConfigOption::lock_while_program_is_running() const{
    return m_data->lock_while_program_is_running;
}
std::string ConfigOption::check_validity() const{
    return std::string();
}
void ConfigOption::restore_defaults(){}
ConfigOptionState ConfigOption::visibility() const{
    return m_data->visibility.load(std::memory_order_relaxed);
}
void ConfigOption::set_visibility(ConfigOptionState visibility){
    if (m_data->set_visibility(visibility)){
        report_visibility_changed();
    }
}


void ConfigOption::report_visibility_changed(){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->visibility_changed();
    }
}
void ConfigOption::report_program_state(bool program_is_running){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->program_state_changed(program_is_running);
    }
}
void ConfigOption::report_value_changed(){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->value_changed();
    }
}





}
