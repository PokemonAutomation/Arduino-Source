/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "BooleanCheckBoxWidget.h"

namespace PokemonAutomation{




ConfigWidget* BooleanCheckBoxOption::make_ui(QWidget& parent){
    return new BooleanCheckBoxWidget(parent, *this);
}



BooleanCheckBoxWidget::~BooleanCheckBoxWidget(){
    m_value.remove_listener(*this);
}
BooleanCheckBoxWidget::BooleanCheckBoxWidget(QWidget& parent, BooleanCheckBoxOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 3);
    m_box = new QCheckBox(this);
    m_box->setChecked(m_value);
    layout->addWidget(m_box, 1);
    connect(
        m_box, &QCheckBox::stateChanged,
        this, [=](int){
            m_value = m_box->isChecked();
        }
    );
    value.add_listener(*this);
}
void BooleanCheckBoxWidget::update(){
    ConfigWidget::update();
    if (m_value != m_box->isChecked()){
        m_box->setChecked(m_value);
    }
}
void BooleanCheckBoxWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update();
    }, Qt::QueuedConnection);
}





}

