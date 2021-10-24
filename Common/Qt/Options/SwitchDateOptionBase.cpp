/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include "Common/Qt/QtJsonTools.h"
#include "SwitchDateOptionBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchDateOptionBase::SwitchDateOptionBase(
    QString label,
    QDate default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}

SwitchDateOptionBase::operator QDate() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QDate SwitchDateOptionBase::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QString SwitchDateOptionBase::set(QDate x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        SpinLockGuard lg(m_lock);
        m_current = x;
    }
    return err;
}

QString SwitchDateOptionBase::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
QString SwitchDateOptionBase::check_validity(QDate x) const{
    return valid_switch_date(x) ? QString() : "Invalid Switch date.";
}
void SwitchDateOptionBase::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}

void SwitchDateOptionBase::load_default(const QJsonValue& json){
    json_parse_date(m_default, json);
}
void SwitchDateOptionBase::load_current(const QJsonValue& json){
    SpinLockGuard lg(m_lock);
    json_parse_date(m_current, json);
}
QJsonValue SwitchDateOptionBase::write_default() const{
    return json_write_date(m_default);
}
QJsonValue SwitchDateOptionBase::write_current() const{
    SpinLockGuard lg(m_lock);
    return json_write_date(m_current);
}

SwitchDateOptionBaseUI::SwitchDateOptionBaseUI(QWidget& parent, SwitchDateOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    m_date_edit = new QDateEdit(m_value.get());
    layout->addWidget(m_date_edit, 1);
    m_date_edit->setDisplayFormat("MMMM d, yyyy");
    m_date_edit->setMinimumDate(QDate(2000, 1, 1));
    m_date_edit->setMaximumDate(QDate(2060, 12, 31));

    connect(
        m_date_edit, &QDateEdit::dateChanged,
        this, [=](const QDate& date){
            m_value.set(date);
        }
    );
}
void SwitchDateOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    m_date_edit->setDate(m_value.get());
}
    

}
}

