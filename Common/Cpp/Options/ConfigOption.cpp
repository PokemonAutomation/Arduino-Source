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
    : m_listeners(CONSTRUCT_TOKEN)
{}

struct ConfigOption::Listeners{
    SpinLock lock;
    std::set<Listener*> listeners;
};
void ConfigOption::add_listener(Listener& listener){
    Listeners& listeners = *m_listeners;
    SpinLockGuard lg(listeners.lock);
    listeners.listeners.insert(&listener);
}
void ConfigOption::remove_listener(Listener& listener){
    Listeners& listeners = *m_listeners;
    SpinLockGuard lg(listeners.lock);
    listeners.listeners.erase(&listener);
}
void ConfigOption::push_update(){
    Listeners& listeners = *m_listeners;
    SpinLockGuard lg(listeners.lock);
    for (Listener* listener : listeners.listeners){
        listener->value_changed();
    }
}





std::string ConfigOption::check_validity() const{
    return std::string();
}








}
