/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include "SimpleIntegerOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerOptionBase<Type>::SimpleIntegerOptionBase(
    QString label,
    Type min_value,
    Type max_value,
    Type default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

template <typename Type>
QString SimpleIntegerOptionBase<Type>::set(Type x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        m_current.store(x, std::memory_order_relaxed);
    }
    return err;
}

template <typename Type>
QString SimpleIntegerOptionBase<Type>::check_validity(Type x) const{
    if (x < m_min_value){
        return "Value too small: min = " + QString::number(m_min_value) + ", value = " + QString::number(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + QString::number(m_max_value) + ", value = " + QString::number(x);
    }
    return QString();
}
template <typename Type>
void SimpleIntegerOptionBase<Type>::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}

template <typename Type>
void SimpleIntegerOptionBase<Type>::load_default(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    m_default = json.toInt();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
template <typename Type>
void SimpleIntegerOptionBase<Type>::load_current(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    Type current = json.toInt();
    current = std::max(current, m_min_value);
    current = std::min(current, m_max_value);
    m_current.store(current, std::memory_order_relaxed);
}
template <typename Type>
QJsonValue SimpleIntegerOptionBase<Type>::write_default() const{
    return QJsonValue((qint64)m_default);
}
template <typename Type>
QJsonValue SimpleIntegerOptionBase<Type>::write_current() const{
    return QJsonValue((qint64)m_current);
}

template <typename Type>
QString SimpleIntegerOptionBase<Type>::check_validity() const{
    Type current = m_current.load(std::memory_order_relaxed);
    return check_validity(current);
}


template <typename Type>
SimpleIntegerOptionBaseUI<Type>::SimpleIntegerOptionBaseUI(QWidget& parent, SimpleIntegerOptionBase<Type>& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new QLineEdit(QString::number(m_value), this);
//    box->setInputMask("999999999");
//    QIntValidator* validator = new QIntValidator(value.m_min_value, value.m_max_value, box);
//    box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            Type current = (Type)text.toLong(&ok);
            QPalette palette;
            if (ok && m_value.set(current).isEmpty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            m_box->setPalette(palette);
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            m_box->setText(QString::number(m_value));
        }
    );
}
template <typename Type>
void SimpleIntegerOptionBaseUI<Type>::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(QString::number(m_value));
}





template class SimpleIntegerOptionBase<uint8_t>;
template class SimpleIntegerOptionBase<uint16_t>;
template class SimpleIntegerOptionBase<uint32_t>;
template class SimpleIntegerOptionBase<uint64_t>;
template class SimpleIntegerOptionBase<int8_t>;

template class SimpleIntegerOptionBaseUI<uint8_t>;
template class SimpleIntegerOptionBaseUI<uint16_t>;
template class SimpleIntegerOptionBaseUI<uint32_t>;
template class SimpleIntegerOptionBaseUI<uint64_t>;
template class SimpleIntegerOptionBaseUI<int8_t>;

}
