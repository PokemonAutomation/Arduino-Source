/*  Simple Integer Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{


template class SimpleIntegerOption<uint8_t>;
template class SimpleIntegerOption<uint16_t>;
template class SimpleIntegerOption<uint32_t>;

template class SimpleIntegerOptionUI<uint8_t>;
template class SimpleIntegerOptionUI<uint16_t>;
template class SimpleIntegerOptionUI<uint32_t>;


template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    Type& backing,
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
    , m_backing(backing)
{}
template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    QString label,
    Type min_value,
    Type max_value,
    Type default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(m_backing)
    , m_backing(default_value)
{}

template <typename Type>
void SimpleIntegerOption<Type>::load_default(const QJsonValue& json){
    m_default = json.toInt();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
template <typename Type>
void SimpleIntegerOption<Type>::load_current(const QJsonValue& json){
    m_current = json.toInt();
    m_current = std::max(m_current, m_min_value);
    m_current = std::min(m_current, m_max_value);
}
template <typename Type>
QJsonValue SimpleIntegerOption<Type>::write_default() const{
    return QJsonValue((qint64)m_default);
}
template <typename Type>
QJsonValue SimpleIntegerOption<Type>::write_current() const{
    return QJsonValue((qint64)m_current);
}

template <typename Type>
bool SimpleIntegerOption<Type>::is_valid() const{
    return m_current >= m_min_value && m_current <= m_max_value;
}
template <typename Type>
void SimpleIntegerOption<Type>::restore_defaults(){
    m_current = m_default;
}


template <typename Type>
SimpleIntegerOptionUI<Type>::SimpleIntegerOptionUI(QWidget& parent, SimpleIntegerOption<Type>& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new QLineEdit(QString::number(m_value), this);
//    box->setInputMask("999999999");
//    QIntValidator* validator = new QIValidator(value.m_min_value, value.m_max_value, box);
//    box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            int64_t read = text.toLong(&ok);
            int64_t fixed = read;
//            cout << "read = " << read << endl;
            fixed = std::max(fixed, (int64_t)m_value.m_min_value);
            fixed = std::min(fixed, (int64_t)m_value.m_max_value);
            if (read != fixed){
                m_box->setText(QString::number(fixed));
            }
//            cout << "fixed = " << fixed << endl;
            m_value.m_current = (Type)fixed;
            QPalette *palette = new QPalette();
            palette->setColor(QPalette::Text, ok && m_value.is_valid() ? Qt::black : Qt::red);
            m_box->setPalette(*palette);
        }
    );
}
template <typename Type>
void SimpleIntegerOptionUI<Type>::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(QString::number(m_value));
}


}
