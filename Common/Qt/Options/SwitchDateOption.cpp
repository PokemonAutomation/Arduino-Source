/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include "Common/Qt/QtJsonTools.h"
#include "SwitchDateOption.h"

namespace PokemonAutomation{


SwitchDateOption::SwitchDateOption(
    QString label,
    QDate default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}
void SwitchDateOption::load_default(const QJsonValue& json){
    json_parse_date(m_default, json);
}
void SwitchDateOption::load_current(const QJsonValue& json){
    json_parse_date(m_current, json);
}
QJsonValue SwitchDateOption::write_default() const{
    return json_write_date(m_default);
}
QJsonValue SwitchDateOption::write_current() const{
    return json_write_date(m_current);
}

bool SwitchDateOption::is_valid() const{
    return valid_switch_date(m_current);
}
void SwitchDateOption::restore_defaults(){
    m_current = m_default;
}

SwitchDateOptionUI::SwitchDateOptionUI(QWidget& parent, SwitchDateOption& value)
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
void SwitchDateOptionUI::restore_defaults(){
    m_value.restore_defaults();
    m_date_edit->setDate(m_value.m_current);
}
    

}

