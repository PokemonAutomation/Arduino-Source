/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Pimpl.tpp"
#include "ConfigOption.h"

namespace PokemonAutomation{



ConfigOption::~ConfigOption() = default;
ConfigOption::ConfigOption()
    : m_data(CONSTRUCT_TOKEN)
{}

struct ConfigOption::Data{
    SpinLock lock;
    std::atomic<ConfigOptionState> visibility;
    std::set<Listener*> listeners;

    Data()
        : visibility(ConfigOptionState::ENABLED)
    {}
};
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
    m_data->visibility.store(visibility, std::memory_order_relaxed);
    push_update();
}








}
