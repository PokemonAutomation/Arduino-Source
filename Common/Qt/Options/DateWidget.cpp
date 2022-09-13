/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include "DateWidget.h"

namespace PokemonAutomation{



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
void DateWidget::update(){
    ConfigWidget::update();
    m_date_edit->setDate(m_value.get());
}
void DateWidget::value_changed(){
    QMetaObject::invokeMethod(m_date_edit, [this]{
        update();
    }, Qt::QueuedConnection);
}



}
