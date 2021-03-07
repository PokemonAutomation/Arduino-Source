/*  Floating-Point Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include "FloatingPointOption.h"

namespace PokemonAutomation{


FloatingPointOption::FloatingPointOption(
    QString label,
    double min_value,
    double max_value,
    double default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

void FloatingPointOption::load_default(const QJsonValue& json){
    m_default = json.toDouble();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
void FloatingPointOption::load_current(const QJsonValue& json){
    m_current = json.toDouble();
    m_current = std::max(m_current, m_min_value);
    m_current = std::min(m_current, m_max_value);
}
QJsonValue FloatingPointOption::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue FloatingPointOption::write_current() const{
    return QJsonValue(m_current);
}

bool FloatingPointOption::is_valid() const{
    return m_current >= m_min_value && m_current <= m_max_value;
}
void FloatingPointOption::restore_defaults(){
    m_current = m_default;
}


FloatingPointOptionUI::FloatingPointOptionUI(QWidget& parent, FloatingPointOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new QLineEdit(QString::number(m_value, 'f', 2), this);
//    box->setInputMask("999999999");
    QDoubleValidator* validator = new QDoubleValidator(value.m_min_value, value.m_max_value, 2, this);
    m_box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            double read = text.toDouble();
            double fixed = read;
//            fixed = std::max(fixed, m_value.m_min_value);
            fixed = std::min(fixed, m_value.m_max_value);
            if (read != fixed){
                m_box->setText(QString::number(fixed, 'f', 2));
            }
            m_value.m_current = fixed;
            QPalette *palette = new QPalette();
            palette->setColor(QPalette::Text, m_value.is_valid() ? Qt::black : Qt::red);
            m_box->setPalette(*palette);
        }
    );
}
void FloatingPointOptionUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(QString::number(m_value, 'f', 2));
}


}
