/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
        this, &QLineEdit::textChanged,
        this, [this](const QString& text){
            std::string error = m_value.set(text.toStdString());
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void TimeDurationCellWidget<Type>::update_value(){
    this->setText(QString::fromStdString(m_value.current_text()));
}
template <typename Type>
void TimeDurationCellWidget<Type>::value_changed(void* object){
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
    row0->addWidget(new QLabel(QString::fromStdString(value.units()), this));

    QLabel* description = new QLabel(QString::fromStdString(m_value.time_string()), this);
    description->setAlignment(Qt::AlignHCenter);
    row1->addWidget(description);

    connect(
        m_box, &QLineEdit::editingFinished,
        this, [this, description](){
            std::string error = m_value.set(m_box->text().toStdString());
            if (error.empty()){
                description->setText(QString::fromStdString(m_value.time_string()));
            }else{
                description->setText(QString::fromStdString("<font color=\"red\">" + error + "</font>"));
            }
        }
    );
    connect(
        m_box, &QLineEdit::textChanged,
        this, [this, description](){
            std::string text = m_value.time_string(m_box->text().toStdString());
            description->setText(QString::fromStdString(text));
        }
    );

    value.add_listener(*this);
}
template <typename Type>
void TimeDurationOptionWidget<Type>::update_value(){
    m_box->setText(QString::fromStdString(m_value.current_text()));
}
template <typename Type>
void TimeDurationOptionWidget<Type>::value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        this->update_value();
    }, Qt::QueuedConnection);
}





template class TimeDurationCellWidget<std::chrono::milliseconds>;
template class TimeDurationOptionWidget<std::chrono::milliseconds>;




}
