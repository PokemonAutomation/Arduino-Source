/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "StringWidget.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{


ConfigWidget* StringCell::make_QtWidget(QWidget& parent){
    return new StringCellWidget(parent, *this);
}

ConfigWidget* StringOption::make_QtWidget(QWidget& parent){
    return new StringOptionWidget(parent, *this);
}



StringCellWidget::~StringCellWidget(){
    m_value.remove_listener(*this);
}
StringCellWidget::StringCellWidget(QWidget& parent, StringCell& value)
    : QLineEdit(QString::fromStdString(value), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    this->setPlaceholderText(QString::fromStdString(value.placeholder_text()));

    this->setReadOnly(value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
    if (m_value.is_password()){
        this->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }

    connect(
        this, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(this->text().toStdString());
        }
    );
    if (m_value.signal_all_text_changes()){
        connect(
            this, &QLineEdit::textChanged,
            [this]{
                std::string old_value = (std::string)m_value;
                std::string text = this->text().toStdString();
                if (old_value == text){
                    return;
                }
                m_value.set(std::move(text));
            }
        );
    }

    m_value.add_listener(*this);
}
void StringCellWidget::update_value(){
    this->setText(QString::fromStdString(m_value));
}
void StringCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void StringCellWidget::on_config_visibility_changed(){
    // Overwrite ConfigWidget::on_config_visibility_changed() because ConfigWidget cannot handle
    // READ_ONLY state.
    this->setEnabled(true);
    QMetaObject::invokeMethod(this, [this]{
        const ConfigOptionState visibility = m_value.visibility();
        this->setReadOnly(visibility != ConfigOptionState::ENABLED || 
            m_value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
        switch (visibility){
        case ConfigOptionState::ENABLED:
        case ConfigOptionState::DISABLED:
            this->setVisible(true);
            break;
        case ConfigOptionState::HIDDEN:
            this->setVisible(false);
            break;
        }
    }, Qt::QueuedConnection);
}




StringOptionWidget::~StringOptionWidget(){
    m_value.remove_listener(*this);
}
StringOptionWidget::StringOptionWidget(QWidget& parent, StringOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    m_box = new QLineEdit(QString::fromStdString(m_value), this);
    m_box->setPlaceholderText(QString::fromStdString(value.placeholder_text()));
    layout->addWidget(m_box, 1);

    m_box->setReadOnly(value.lock_mode() == LockMode::READ_ONLY);
    if (m_value.is_password()){
        m_box->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }

    connect(
        m_box, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(m_box->text().toStdString());
        }
    );
    if (m_value.signal_all_text_changes()){
        connect(
            m_box, &QLineEdit::textChanged,
            [this]{
                const std::string old_value = (std::string)m_value;
                std::string text = m_box->text().toStdString();
                if (old_value == text){
                    return;
                }
                m_value.set(std::move(text));
            }
        );
    }

    m_value.add_listener(*this);
}
void StringOptionWidget::update_value(){
    m_box->setText(QString::fromStdString(m_value));
}
void StringOptionWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void StringOptionWidget::on_config_visibility_changed(){
    // Overwrite ConfigWidget::on_config_visibility_changed() because ConfigWidget cannot handle
    // READ_ONLY state.
    this->setEnabled(true);
    QMetaObject::invokeMethod(this, [this]{
        const ConfigOptionState visibility = m_value.visibility();
        m_box->setReadOnly(visibility != ConfigOptionState::ENABLED || 
            m_value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
        switch (visibility){
        case ConfigOptionState::ENABLED:
        case ConfigOptionState::DISABLED:
            this->setVisible(true);
            break;
        case ConfigOptionState::HIDDEN:
            this->setVisible(false);
            break;
        }
    }, Qt::QueuedConnection);
}





}
