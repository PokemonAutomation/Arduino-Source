/*  Integer Range Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "ConfigWidget.h"
#include "IntegerRangeWidget.h"

namespace PokemonAutomation{


template <typename Type>
IntegerRangeCellWidget<Type>::~IntegerRangeCellWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
IntegerRangeCellWidget<Type>::IntegerRangeCellWidget(QWidget& parent, IntegerRangeCell<Type>& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    m_lo = new QLineEdit(this);
    m_hi = new QLineEdit(this);
    layout->addWidget(m_lo);
    layout->addWidget(new QLabel(" - ", this));
    layout->addWidget(m_hi);

    Type lo, hi;
    value.current_values(lo, hi);
    m_lo->setText(QString::number(lo));
    m_hi->setText(QString::number(hi));

    if (sizeof(Type) <= 1){
        m_lo->setMaximumWidth(30);
        m_hi->setMaximumWidth(30);
    }

    connect(
        m_lo, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            if (std::is_unsigned_v<Type>){
                uint64_t current = m_lo->text().toULongLong(&ok);
                m_value.set_lo((Type)current);
            }else{
                uint64_t current = m_lo->text().toLongLong(&ok);
                m_value.set_lo((Type)current);
            }
        }
    );
    connect(
        m_hi, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            if (std::is_unsigned_v<Type>){
                uint64_t current = m_hi->text().toULongLong(&ok);
                m_value.set_hi((Type)current);
            }else{
                uint64_t current = m_hi->text().toLongLong(&ok);
                m_value.set_hi((Type)current);
            }
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void IntegerRangeCellWidget<Type>::update_value(){
    Type lo, hi;
    m_value.current_values(lo, hi);
    m_lo->setText(QString::number(lo));
    m_hi->setText(QString::number(hi));
}
template <typename Type>
void IntegerRangeCellWidget<Type>::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



template class IntegerRangeCellWidget<uint8_t>;




}
