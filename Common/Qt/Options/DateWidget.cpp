/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include "DateWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




ConfigWidget* DateTimeCell::make_QtWidget(QWidget& parent){
    return new DateTimeCellWidget(parent, *this);
}
ConfigWidget* DateTimeOption::make_QtWidget(QWidget& parent){
    return new DateTimeOptionWidget(parent, *this);
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



DateTimeCellWidget::~DateTimeCellWidget(){
    m_value.remove_listener(*this);
}
DateTimeCellWidget::DateTimeCellWidget(QWidget& parent, DateTimeCell& value)
    : QDateTimeEdit(DateTime_to_QDateTime(value.get()), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    switch (value.level()){
    case DateTimeOption::DATE:
        this->setDisplayFormat("MMMM d, yyyy");
        break;
    case DateTimeOption::DATE_HOUR_MIN:
        this->setDisplayFormat("MMMM d, yyyy hh:mm");
        break;
    case DateTimeOption::DATE_HOUR_MIN_SEC:
        this->setDisplayFormat("MMMM d, yyyy hh:mm:ss");
        break;
    }
    this->setMinimumDateTime(DateTime_to_QDateTime(value.min_value()));
    this->setMaximumDateTime(DateTime_to_QDateTime(value.max_value()));

//    cout << "Max time = " << m_date_edit->maximumTime().toString().toStdString() << endl;

    connect(
        this, &QDateTimeEdit::dateTimeChanged,
        this, [this](const QDateTime& date){
            m_value.set(QDateTime_to_DateTime(date, m_value.level()));
        }
    );

    value.add_listener(*this);
}
void DateTimeCellWidget::update_value(){
    this->setDateTime(DateTime_to_QDateTime(m_value.get()));
}
void DateTimeCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



DateTimeOptionWidget::DateTimeOptionWidget(QWidget& parent, DateTimeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);

    m_date_edit = new DateTimeCellWidget(*this, value);
    layout->addWidget(m_date_edit, 1);
}





}
