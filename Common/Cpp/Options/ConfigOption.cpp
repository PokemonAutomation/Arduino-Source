/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


struct ConfigOption::Data{
    SpinLock lock;
    std::atomic<ConfigOptionState> visibility;
    std::set<Listener*> listeners;

    Data(ConfigOptionState p_visibility = ConfigOptionState::ENABLED)
        : visibility(p_visibility)
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
    : m_data(CONSTRUCT_TOKEN, x.visibility())
{}

ConfigOption::ConfigOption()
    : m_data(CONSTRUCT_TOKEN)
{}
ConfigOption::ConfigOption(ConfigOptionState visibility)
    : m_data(CONSTRUCT_TOKEN, visibility)
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
void ConfigOption::push_update(){
    Data& data = *m_data;
    SpinLockGuard lg(data.lock);
    for (Listener* listener : data.listeners){
        listener->value_changed();
    }
}





std::string ConfigOption::check_validity() const{
    return std::string();
}
ConfigOptionState ConfigOption::visibility() const{
    return m_data->visibility.load(std::memory_order_relaxed);
}
void ConfigOption::set_visibility(ConfigOptionState visibility){
    if (m_data->set_visibility(visibility)){
        push_update();
    }
}








}
