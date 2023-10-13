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
    LockMode lock_mode;
    std::atomic<ConfigOptionState> visibility;
    std::set<Listener*> listeners;
    
    Data(LockMode p_lock_mode, ConfigOptionState p_visibility)
        : lock_mode(p_lock_mode)
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
    : m_data(CONSTRUCT_TOKEN, x.lock_mode(), x.visibility())
{}

ConfigOption::ConfigOption()
    : m_data(CONSTRUCT_TOKEN, LockMode::LOCK_WHILE_RUNNING, ConfigOptionState::ENABLED)
{}
ConfigOption::ConfigOption(LockMode lock_mode)
    : m_data(CONSTRUCT_TOKEN, lock_mode, ConfigOptionState::ENABLED)
{}
ConfigOption::ConfigOption(ConfigOptionState visibility)
    : m_data(CONSTRUCT_TOKEN, LockMode::LOCK_WHILE_RUNNING, visibility)
{}

void ConfigOption::add_listener(Listener& listener){
    m_lifetime_sanitizer.check_usage();
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    data.listeners.insert(&listener);
}
void ConfigOption::remove_listener(Listener& listener){
    m_lifetime_sanitizer.check_usage();
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    data.listeners.erase(&listener);
}
size_t ConfigOption::total_listeners() const{
    m_lifetime_sanitizer.check_usage();
    const Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    return data.listeners.size();
}





void ConfigOption::load_json(const JsonValue& json){
    m_lifetime_sanitizer.check_usage();
}
JsonValue ConfigOption::to_json() const{
    m_lifetime_sanitizer.check_usage();
    return JsonValue();
}
LockMode ConfigOption::lock_mode() const{
    m_lifetime_sanitizer.check_usage();
    return m_data->lock_mode;
}
std::string ConfigOption::check_validity() const{
    m_lifetime_sanitizer.check_usage();
    return std::string();
}
void ConfigOption::restore_defaults(){
    m_lifetime_sanitizer.check_usage();
}
ConfigOptionState ConfigOption::visibility() const{
    m_lifetime_sanitizer.check_usage();
    return m_data->visibility.load(std::memory_order_relaxed);
}
void ConfigOption::set_visibility(ConfigOptionState visibility){
    m_lifetime_sanitizer.check_usage();
    if (m_data->set_visibility(visibility)){
        report_visibility_changed();
    }
}


void ConfigOption::report_visibility_changed(){
    m_lifetime_sanitizer.check_usage();
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->visibility_changed();
    }
}
void ConfigOption::report_program_state(bool program_is_running){
    m_lifetime_sanitizer.check_usage();
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->program_state_changed(program_is_running);
    }
}
void ConfigOption::report_value_changed(){
    m_lifetime_sanitizer.check_usage();
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
//    cout << "listeners = " << data.listeners.size() << endl;
    for (Listener* listener : data.listeners){
        listener->value_changed();
    }
}





}
