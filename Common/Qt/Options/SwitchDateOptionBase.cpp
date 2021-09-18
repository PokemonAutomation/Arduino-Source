/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include "Common/Qt/QtJsonTools.h"
#include "SwitchDateOptionBase.h"

namespace PokemonAutomation{


SwitchDateOptionBase::SwitchDateOptionBase(
    QString label,
    QDate default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}
void SwitchDateOptionBase::load_default(const QJsonValue& json){
    json_parse_date(m_default, json);
}
void SwitchDateOptionBase::load_current(const QJsonValue& json){
    json_parse_date(m_current, json);
}
QJsonValue SwitchDateOptionBase::write_default() const{
    return json_write_date(m_default);
}
QJsonValue SwitchDateOptionBase::write_current() const{
    return json_write_date(m_current);
}

bool SwitchDateOptionBase::is_valid() const{
    return valid_switch_date(m_current);
}
void SwitchDateOptionBase::restore_defaults(){
    m_current = m_default;
}

SwitchDateOptionBaseUI::SwitchDateOptionBaseUI(QWidget& parent, SwitchDateOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    m_date_edit = new QDateEdit(m_value.m_current);
    layout->addWidget(m_date_edit, 1);
    m_date_edit->setDisplayFormat("MMMM d, yyyy");
    m_date_edit->setMinimumDate(QDate(2000, 1, 1));
    m_date_edit->setMaximumDate(QDate(2060, 12, 31));

    connect(
        m_date_edit, &QDateEdit::dateChanged,
        this, [=](const QDate& date){
            m_value.m_current = date;
        }
    );
}
void SwitchDateOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    m_date_edit->setDate(m_value.m_current);
}
    

}

