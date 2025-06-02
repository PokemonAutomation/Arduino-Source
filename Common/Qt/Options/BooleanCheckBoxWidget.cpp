/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/
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
#if QT_VERSION < 0x060700
    connect(
        m_box, &QCheckBox::stateChanged,
        this, [this](int){
            m_value = m_box->isChecked();
        }
    );
#else
    connect(
        m_box, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState){
            m_value = m_box->isChecked();
        }
    );
#endif
    value.add_listener(*this);
}
void BooleanCheckBoxCellWidget::update_value(){
    if (m_value != m_box->isChecked()){
        m_box->setChecked(m_value);
    }
}
void BooleanCheckBoxCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
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
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 3);
    m_box = new QCheckBox(this);
    m_box->setChecked(m_value);
    layout->addWidget(m_box, 1);
#if QT_VERSION < 0x060700
    connect(
        m_box, &QCheckBox::stateChanged,
        this, [this](int){
            m_value = m_box->isChecked();
        }
    );
#else
    connect(
        m_box, &QCheckBox::checkStateChanged,
        this, [this](int){
            m_value = m_box->isChecked();
        }
    );
#endif
    value.add_listener(*this);
}
void BooleanCheckBoxOptionWidget::update_value(){
    if (m_value != m_box->isChecked()){
        m_box->setChecked(m_value);
    }
}
void BooleanCheckBoxOptionWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





}

