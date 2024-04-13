/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include <QDateTimeEdit>
#include "DateWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



#if 0
ConfigWidget* DateOption::make_QtWidget(QWidget& parent){
    return new DateWidget(parent, *this);
}



DateWidget::~DateWidget(){
    m_value.remove_listener(*this);
}
DateWidget::DateWidget(QWidget& parent, DateOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);

    m_date_edit = new QDateEdit(m_value.get());
    layout->addWidget(m_date_edit, 1);
    m_date_edit->setDisplayFormat("MMMM d, yyyy");
    m_date_edit->setMinimumDate(value.min_value());
    m_date_edit->setMaximumDate(value.max_value());

    connect(
        m_date_edit, &QDateEdit::dateChanged,
        this, [this](const QDate& date){
            m_value.set(date);
        }
    );

    value.add_listener(*this);
}
void DateWidget::update_value(){
    m_date_edit->setDate(m_value.get());
}
void DateWidget::value_changed(){
    QMetaObject::invokeMethod(m_date_edit, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
#endif



ConfigWidget* DateTimeOption::make_QtWidget(QWidget& parent){
    return new DateTimeWidget(parent, *this);
}


QDateTime DateTime_to_QDateTime(const DateTime& date){
    return QDateTime(
        QDate(date.year, date.month, date.day),
        QTime(
            std::max<int>(date.hour, 0),
            std::max<int>(date.minute, 0),
            std::max<int>(date.second, 0)
        )
    );
}
DateTime QDateTime_to_DateTime(const QDateTime& date, DateTimeOption::Level level){
    DateTime ret;
    QDate qdate = date.date();
    ret.year = (int16_t)qdate.year();
    ret.month = (int8_t)qdate.month();
    ret.day = (int8_t)qdate.day();

    if (level < DateTimeOption::DATE_HOUR_MIN){
        return ret;
    }

    QTime qtime = date.time();
    ret.hour = (int8_t)qtime.hour();
    ret.minute = (int8_t)qtime.minute();

    if (level < DateTimeOption::DATE_HOUR_MIN_SEC){
        return ret;
    }

    ret.second = (int8_t)qtime.second();

    return ret;
}


DateTimeWidget::~DateTimeWidget(){
    m_value.remove_listener(*this);
}
DateTimeWidget::DateTimeWidget(QWidget& parent, DateTimeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);

    m_date_edit = new QDateTimeEdit(DateTime_to_QDateTime(m_value.get()));
    layout->addWidget(m_date_edit, 1);
    switch (value.level()){
    case DateTimeOption::DATE:
        m_date_edit->setDisplayFormat("MMMM d, yyyy");
        break;
    case DateTimeOption::DATE_HOUR_MIN:
        m_date_edit->setDisplayFormat("MMMM d, yyyy hh:mm");
        break;
    case DateTimeOption::DATE_HOUR_MIN_SEC:
        m_date_edit->setDisplayFormat("MMMM d, yyyy hh:mm:ss");
        break;
    }
    m_date_edit->setMinimumDateTime(DateTime_to_QDateTime(value.min_value()));
    m_date_edit->setMaximumDateTime(DateTime_to_QDateTime(value.max_value()));

//    cout << "Max time = " << m_date_edit->maximumTime().toString().toStdString() << endl;

    connect(
        m_date_edit, &QDateTimeEdit::dateTimeChanged,
        this, [this](const QDateTime& date){
            m_value.set(QDateTime_to_DateTime(date, m_value.level()));
        }
    );

    value.add_listener(*this);
}
void DateTimeWidget::update_value(){
    m_date_edit->setDateTime(DateTime_to_QDateTime(m_value.get()));
}
void DateTimeWidget::value_changed(){
    QMetaObject::invokeMethod(m_date_edit, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





}
