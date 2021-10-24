/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/NintendoSwitch/NintendoSwitch_Tools.h"
#include "TimeExpressionOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
TimeExpressionOptionBase<Type>::TimeExpressionOptionBase(
    QString label,
    Type min_value,
    Type max_value,
    QString default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(std::move(default_value))
    , m_current(m_default)
{
    m_error = process(m_current, m_value);
}

template <typename Type>
TimeExpressionOptionBase<Type>::operator Type() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
Type TimeExpressionOptionBase<Type>::get() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
QString TimeExpressionOptionBase<Type>::set(QString text){
    Type value;
    QString error = process(text, value);
    if (error.isEmpty()){
        SpinLockGuard lg(m_lock);
        m_current = std::move(text);
        m_value = value;
        m_error.clear();
    }
    return error;
}

template <typename Type>
QString TimeExpressionOptionBase<Type>::text() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
template <typename Type>
QString TimeExpressionOptionBase<Type>::time_string() const{
    SpinLockGuard lg(m_lock);
    if (!m_error.isEmpty()){
        return "<font color=\"red\">" + m_error + "</font>";
    }
    return QString::fromStdString(NintendoSwitch::ticks_to_time(m_value));
}

template <typename Type>
void TimeExpressionOptionBase<Type>::load_default(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    m_default = json.toString();
    m_error = process(m_current, m_value);
}
template <typename Type>
void TimeExpressionOptionBase<Type>::load_current(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
    m_error = process(m_current, m_value);
}
template <typename Type>
QJsonValue TimeExpressionOptionBase<Type>::write_default() const{
    return QJsonValue(m_default);
}
template <typename Type>
QJsonValue TimeExpressionOptionBase<Type>::write_current() const{
    SpinLockGuard lg(m_lock);
    return QJsonValue(m_current);
}

template <typename Type>
QString TimeExpressionOptionBase<Type>::check_validity() const{
    SpinLockGuard lg(m_lock);
    return m_error;
}
template <typename Type>
void TimeExpressionOptionBase<Type>::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
    m_error = process(m_current, m_value);
}


template <typename Type>
QString TimeExpressionOptionBase<Type>::process(const QString& text, Type& value) const{
    if (text.isEmpty() || text.isNull()){
        return "Expression is empty.";
    }
    uint32_t parsed;
    try{
        parsed = parse_ticks_i32(text);
    }catch (const ParseException& str){
        return str.message_qt();
    }
//    cout << "value = " << parsed << endl;

    if (parsed < m_min_value){
        return "Overflow: Number is too small.";
    }
    if (parsed > m_max_value){
        return "Overflow: Number is too large.";
    }
    value = (Type)parsed;
    return QString();
}
template <typename Type>
TimeExpressionOptionBaseUI<Type>::TimeExpressionOptionBaseUI(QWidget& parent, TimeExpressionOptionBase<Type>& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.label(), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    QVBoxLayout* rows = new QVBoxLayout();
    layout->addLayout(rows, 1);

    QHBoxLayout* row0 = new QHBoxLayout();
    rows->addLayout(row0);
    QHBoxLayout* row1 = new QHBoxLayout();
    rows->addLayout(row1);

    m_box = new QLineEdit(m_value.text(), this);
    row0->addWidget(m_box);
    row0->addWidget(new QLabel("ticks", this));

    QLabel* seconds = new QLabel(m_value.time_string(), this);
    seconds->setAlignment(Qt::AlignHCenter);
    row1->addWidget(seconds);

    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
//           cout << text.toStdString() << endl;
            QString error = m_value.set(text);
            if (error.isEmpty()){
                seconds->setText(m_value.time_string());
            }else{
                seconds->setText("<font color=\"red\">" + error + "</font>");
            }
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        this, [=](){
            m_box->setText(m_value.text());
            seconds->setText(m_value.time_string());
        }
    );
}
template <typename Type>
void TimeExpressionOptionBaseUI<Type>::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(m_value.time_string());
}





template class TimeExpressionOptionBase<uint16_t>;
template class TimeExpressionOptionBase<uint32_t>;

template class TimeExpressionOptionBaseUI<uint16_t>;
template class TimeExpressionOptionBaseUI<uint32_t>;


}
}
