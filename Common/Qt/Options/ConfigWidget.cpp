/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/
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
    auto scope = m_value.check_scope();
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
        // setEnable(false) only happens when the lock mode is LOCK_WHILE_RUNNING and the program is running.
        // Ideally we should handle lock mode READ_ONLY here too, but m_widget as a QWidget does not have such
        // function. Only some of its derived classes, like QLineEdit, can call setReadOnly().
        // So here we treat READ_ONLY as enabled and let derived classes of ConfigWidget that have the
        // functionality to set read-only (e.g. StringOptionWidget) to handle READ_ONLY.
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
    auto scope = m_value.check_scope();
    m_program_is_running = program_is_running;
    update_visibility();
}
void ConfigWidget::update_all(bool program_is_running){
    auto scope = m_value.check_scope();
    update_value();
    update_visibility(program_is_running);
}

void ConfigWidget::on_config_visibility_changed(){
    auto scope = m_value.check_scope();
    QMetaObject::invokeMethod(m_widget, [this]{
        update_visibility();
    }, Qt::QueuedConnection);
}
void ConfigWidget::on_program_state_changed(bool program_is_running){
    auto scope = m_value.check_scope();
    QMetaObject::invokeMethod(m_widget, [this, program_is_running]{
        update_visibility(program_is_running);
    }, Qt::QueuedConnection);
}




}
