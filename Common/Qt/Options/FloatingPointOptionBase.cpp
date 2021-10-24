/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include "FloatingPointOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


FloatingPointOptionBase::FloatingPointOptionBase(
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

QString FloatingPointOptionBase::set(double x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        m_current.store(x, std::memory_order_relaxed);
    }
    return err;
}
void FloatingPointOptionBase::restore_defaults(){
    m_current = m_default;
}

void FloatingPointOptionBase::load_default(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    m_default = json.toDouble();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
void FloatingPointOptionBase::load_current(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    double current = json.toDouble();
    current = std::max(current, m_min_value);
    current = std::min(current, m_max_value);
    m_current.store(current, std::memory_order_relaxed);
}
QJsonValue FloatingPointOptionBase::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue FloatingPointOptionBase::write_current() const{
    return QJsonValue(m_current.load(std::memory_order_relaxed));
}

QString FloatingPointOptionBase::check_validity() const{
    return check_validity(m_current.load(std::memory_order_relaxed));
}
QString FloatingPointOptionBase::check_validity(double x) const{
    if (x < m_min_value){
        return "Value too small: min = " + QString::number(m_min_value) + ", value = " + QString::number(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + QString::number(m_max_value) + ", value = " + QString::number(x);
    }
    if (std::isnan(x)){
        return "Value is NaN";
    }
    return QString();
}


FloatingPointOptionBaseUI::FloatingPointOptionBaseUI(QWidget& parent, FloatingPointOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new QLineEdit(QString::number(m_value, 'f', 2), this);
//    box->setInputMask("999999999");
//    QDoubleValidator* validator = new QDoubleValidator(value.min_value(), value.max_value(), 2, this);
//    m_box->setValidator(validator);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            bool ok;
            double current = text.toDouble(&ok);
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
            m_box->setText(QString::number(m_value, 'f', 2));
        }
    );
}
void FloatingPointOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(QString::number(m_value, 'f', 2));
}


}
