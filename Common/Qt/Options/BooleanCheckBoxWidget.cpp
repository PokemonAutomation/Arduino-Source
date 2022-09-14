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




ConfigWidget* BooleanCheckBoxCell::make_QtWidget(QWidget& parent){
    return new BooleanCheckBoxCellWidget(parent, *this);
}
ConfigWidget* BooleanCheckBoxOption::make_QtWidget(QWidget& parent){
    return new BooleanCheckBoxOptionWidget(parent, *this);
}




BooleanCheckBoxCellWidget::~BooleanCheckBoxCellWidget(){
    m_value.remove_listener(*this);
}
BooleanCheckBoxCellWidget::BooleanCheckBoxCellWidget(QWidget& parent, BooleanCheckBoxCell& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    m_box = new QCheckBox(this);
    m_box->setChecked(m_value);
    layout->addWidget(m_box);
    connect(
        m_box, &QCheckBox::stateChanged,
        this, [this](int){
            m_value = m_box->isChecked();
        }
    );
    value.add_listener(*this);
}
void BooleanCheckBoxCellWidget::update(){
    ConfigWidget::update();
    if (m_value != m_box->isChecked()){
        m_box->setChecked(m_value);
    }
}
void BooleanCheckBoxCellWidget::value_changed(){
    QMetaObject::invokeMethod(this, [this]{
        update();
    }, Qt::QueuedConnection);
}






BooleanCheckBoxOptionWidget::~BooleanCheckBoxOptionWidget(){
    m_value.remove_listener(*this);
}
BooleanCheckBoxOptionWidget::BooleanCheckBoxOptionWidget(QWidget& parent, BooleanCheckBoxOption& value)
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
        this, [this](int){
            m_value = m_box->isChecked();
        }
    );
    value.add_listener(*this);
}
void BooleanCheckBoxOptionWidget::update(){
    ConfigWidget::update();
    if (m_value != m_box->isChecked()){
        m_box->setChecked(m_value);
    }
}
void BooleanCheckBoxOptionWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [this]{
        update();
    }, Qt::QueuedConnection);
}





}

