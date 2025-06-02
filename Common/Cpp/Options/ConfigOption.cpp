/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "ConfigOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct ConfigOption::Data{
    const LockMode lock_mode;
    std::atomic<ConfigOptionState> visibility;

    ListenerSet<Listener> listeners;
    
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
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.listeners.add(listener);
}
void ConfigOption::remove_listener(Listener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.listeners.remove(listener);
}
size_t ConfigOption::total_listeners() const{
    auto scope = m_lifetime_sanitizer.check_scope();
    const Data& data = *m_data;
    return data.listeners.count_unique();
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
    auto scope = m_lifetime_sanitizer.check_scope();
    if (m_data->set_visibility(visibility)){
        report_visibility_changed();
    }
}


void ConfigOption::report_visibility_changed(){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.listeners.run_method_unique(&Listener::on_config_visibility_changed);
}
void ConfigOption::report_program_state(bool program_is_running){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.listeners.run_method_unique(&Listener::on_program_state_changed, program_is_running);
}
void ConfigOption::report_value_changed(void* object){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.listeners.run_method_unique(&Listener::on_config_value_changed, object);
}





}
