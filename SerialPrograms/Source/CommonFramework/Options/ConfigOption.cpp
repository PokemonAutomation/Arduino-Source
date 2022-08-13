/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QWidget>
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


ConfigWidget::ConfigWidget(ConfigOption& m_value)
    : m_value(m_value)
{
    ConfigWidget::update_visibility();
}
ConfigWidget::ConfigWidget(ConfigOption& m_value, QWidget& widget)
    : m_value(m_value)
    , m_widget(&widget)
{
    ConfigWidget::update_visibility();
}
void ConfigWidget::update_ui(){
    update_visibility();
}
void ConfigWidget::update_visibility(){
    if (m_widget == nullptr){
        return;
    }
    switch (m_value.visibility){
    case ConfigOptionState::ENABLED:
        m_widget->setEnabled(true);
        m_widget->setVisible(true);
        break;
    case ConfigOptionState::DISABLED:
        m_widget->setEnabled(false);
        m_widget->setVisible(true);
        break;
    case ConfigOptionState::HIDDEN:
        m_widget->setEnabled(false);
        m_widget->setVisible(false);
        break;
    }
}






}
