/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include <QHBoxLayout>
#include <QLabel>
#include "StringOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


StringOptionBase::StringOptionBase(
    bool is_password,
    QString label,
    QString default_value,
    QString placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_is_password(is_password)
    , m_current(m_default)
{}
StringOptionBase::operator QString() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QString StringOptionBase::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void StringOptionBase::set(QString x){
    SpinLockGuard lg(m_lock);
    m_current = std::move(x);
}

void StringOptionBase::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}

void StringOptionBase::load_default(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_default = json.toString();
}
void StringOptionBase::load_current(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
}
QJsonValue StringOptionBase::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue StringOptionBase::write_current() const{
    SpinLockGuard lg(m_lock);
    return QJsonValue(m_current);
}

StringOptionBaseUI::StringOptionBaseUI(QWidget& parent, StringOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    m_line_edit = new QLineEdit(m_value.get());
    m_line_edit->setPlaceholderText(value.placeholder_text());
    layout->addWidget(m_line_edit, 1);

    if (m_value.is_password()){
        m_line_edit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    }

    connect(
        m_line_edit, &QLineEdit::editingFinished,
        this, [=](){
            m_value.set(m_line_edit->text());
        }
    );
}
void StringOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    m_line_edit->setText(m_value.get());
}
    


}

