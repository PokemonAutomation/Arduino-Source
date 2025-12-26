/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "TimeDurationWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



template <typename Type>
TimeDurationCellWidget<Type>::~TimeDurationCellWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
TimeDurationCellWidget<Type>::TimeDurationCellWidget(QWidget& parent, TimeDurationCell<Type>& value)
    : QLineEdit(QString::fromStdString(value.current_text()), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    this->setToolTip(QString::fromStdString(
        "Time duration in " + value.units() +
        "<br><br>"
        "For controller timings, this will be rounded "
        "up to the tick size of the controller."
        "<br><br>"
        "The tick size for wired controllers is usually 8ms and are very precise."
        "<br><br>"
        "Wireless controllers have larger tick sizes and are imprecise due to wireless communication latency."
    ));

    connect(
        this, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(this->text().toStdString());
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void TimeDurationCellWidget<Type>::update_value(){
    this->setText(QString::fromStdString(m_value.current_text()));
}
template <typename Type>
void TimeDurationCellWidget<Type>::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        this->update_value();
    }, Qt::QueuedConnection);
}





template <typename Type>
TimeDurationOptionWidget<Type>::~TimeDurationOptionWidget(){
    m_value.remove_listener(*this);
}
template <typename Type>
TimeDurationOptionWidget<Type>::TimeDurationOptionWidget(QWidget& parent, TimeDurationOption<Type>& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);
    QVBoxLayout* rows = new QVBoxLayout();
    layout->addLayout(rows, 1);

    QHBoxLayout* row0 = new QHBoxLayout();
    rows->addLayout(row0);
    QHBoxLayout* row1 = new QHBoxLayout();
    rows->addLayout(row1);

    m_box = new QLineEdit(QString::fromStdString(m_value.current_text()), this);
    row0->addWidget(m_box);
    if (value.show_summary()){
        row0->addWidget(new QLabel(QString::fromStdString(value.units()), this));
    }
    m_box->setToolTip(QString::fromStdString(
        "Time duration in " + value.units() +
        "<br><br>"
        "For controller timings, this will be rounded "
        "up to the tick size of the controller."
        "<br><br>"
        "The tick size for wired controllers is usually 8ms and are very precise."
        "<br><br>"
        "Wireless controllers have larger tick sizes and are imprecise due to wireless communication latency."
    ));

    m_description = nullptr;
    if (value.show_summary()){
        m_description = new QLabel(QString::fromStdString(m_value.time_string()), this);
        m_description->setAlignment(Qt::AlignHCenter);
        row1->addWidget(m_description);
    }

    connect(
        m_box, &QLineEdit::editingFinished,
        this, [this](){
            std::string error = m_value.set(m_box->text().toStdString());
            if (m_description == nullptr){
                return;
            }
            if (error.empty()){
                m_description->setText(QString::fromStdString(m_value.time_string()));
            }else{
                m_description->setText(QString::fromStdString("<font color=\"red\">" + error + "</font>"));
            }
        }
    );
#if 0
    connect(
        m_box, &QLineEdit::textChanged,
        this, [this, m_description](){
            if (m_description == nullptr){
                return;
            }
            std::string text = m_value.time_string(m_box->text().toStdString());
            m_description->setText(QString::fromStdString(text));
        }
    );
#endif

    value.add_listener(*this);
}
template <typename Type>
void TimeDurationOptionWidget<Type>::update_value(){
    std::string text = m_value.current_text();
    m_box->setText(QString::fromStdString(text));
    m_description->setText(QString::fromStdString(m_value.time_string()));
}
template <typename Type>
void TimeDurationOptionWidget<Type>::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        this->update_value();
    }, Qt::QueuedConnection);
}





template class TimeDurationCellWidget<std::chrono::milliseconds>;
template class TimeDurationCellWidget<std::chrono::microseconds>;
template class TimeDurationOptionWidget<std::chrono::milliseconds>;
template class TimeDurationOptionWidget<std::chrono::microseconds>;


template class RegisterConfigWidget<TimeDurationCellWidget<std::chrono::milliseconds>>;
template class RegisterConfigWidget<TimeDurationCellWidget<std::chrono::microseconds>>;
template class RegisterConfigWidget<TimeDurationOptionWidget<std::chrono::milliseconds>>;
template class RegisterConfigWidget<TimeDurationOptionWidget<std::chrono::microseconds>>;



}
