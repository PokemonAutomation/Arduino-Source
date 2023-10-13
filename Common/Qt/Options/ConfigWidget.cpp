/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QWidget>
#include "ConfigWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ConfigWidget::~ConfigWidget(){
    m_value.remove_listener(*this);
}
ConfigWidget::ConfigWidget(ConfigOption& m_value)
    : m_value(m_value)
{
    m_value.add_listener(*this);
}
ConfigWidget::ConfigWidget(ConfigOption& m_value, QWidget& widget)
    : m_value(m_value)
    , m_widget(&widget)
{
//    cout << "ConfigWidget::ConfigWidget(): " << (int)m_value.visibility() << endl;
    m_program_is_running = false;
    ConfigWidget::update_visibility();
    m_value.add_listener(*this);
}
void ConfigWidget::update_visibility(){
    m_value.check_usage();
    if (m_widget == nullptr){
        return;
    }
//    cout << "update_visibility = " << program_is_running << endl;
//    if (!m_program_is_running){
//        cout << "asdf" << endl;
//    }
//    cout << "lock_while_program_is_running = " << m_value.lock_while_program_is_running() << endl;
//    cout << "program_is_running = " << m_program_is_running << endl;
//    cout << "ConfigWidget::update_visibility(): " << (int)m_value.visibility() << endl;
    switch (m_value.visibility()){
    case ConfigOptionState::ENABLED:
        m_widget->setEnabled(m_value.lock_mode() != LockMode::LOCK_WHILE_RUNNING || !m_program_is_running);
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
void ConfigWidget::update_visibility(bool program_is_running){
    m_value.check_usage();
    m_program_is_running = program_is_running;
    update_visibility();
}
void ConfigWidget::update_all(bool program_is_running){
    m_value.check_usage();
    update_value();
    update_visibility(program_is_running);
}

void ConfigWidget::visibility_changed(){
    m_value.check_usage();
    QMetaObject::invokeMethod(m_widget, [this]{
        update_visibility();
    }, Qt::QueuedConnection);
}
void ConfigWidget::program_state_changed(bool program_is_running){
    m_value.check_usage();
    QMetaObject::invokeMethod(m_widget, [this, program_is_running]{
        update_visibility(program_is_running);
    }, Qt::QueuedConnection);
}




}
