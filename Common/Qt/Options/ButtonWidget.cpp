/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "ButtonWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ConfigWidget* ButtonCell::make_QtWidget(QWidget& parent){
    return new ButtonCellWidget(parent, *this);
}
ConfigWidget* ButtonOption::make_QtWidget(QWidget& parent){
    return new ButtonOptionWidget(parent, *this);
}


ButtonCellWidget::~ButtonCellWidget(){
    m_value.remove_listener(*this);
}
ButtonCellWidget::ButtonCellWidget(QWidget& parent, ButtonCell& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_button(new QPushButton(this))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_button);

    ButtonCellWidget::update_value();

//    cout << "height = " << this->height() << endl;

    connect(
        m_button, &QPushButton::pressed,
        this, [this]{
            m_value.press_button();
//            cout << "Press: " << this->font().pointSize() << endl;
//            cout << "height = " << this->height() << endl;
        }
    );
    value.add_listener(*this);
}

void ButtonCellWidget::update_value(){
    m_button->setText(QString::fromStdString(m_value.text()));

    int button_size = m_value.button_height();
    if (button_size > 0){
        m_button->setFixedHeight(button_size);
    }

    int text_size = m_value.text_size();
    if (text_size > 0){
        QFont font = m_button->font();
        font.setPointSize(text_size);
//        font.setPointSize(16);
        m_button->setFont(font);
//        cout << "Post set: " << this->font().pointSize() << endl;
    }
//    cout << "Update: " << this->font().pointSize() << endl;
}
void ButtonCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}






ButtonOptionWidget::~ButtonOptionWidget(){
    m_value.remove_listener(*this);
}
ButtonOptionWidget::ButtonOptionWidget(QWidget& parent, ButtonOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_label(new QLabel(this))
    , m_button(new QPushButton(this))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label->setWordWrap(true);
    m_label->setTextFormat(Qt::RichText);
    m_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_label->setOpenExternalLinks(true);
    layout->addWidget(m_label);

    layout->addWidget(m_button);

    ButtonOptionWidget::update_value();

//    cout << "height = " << this->height() << endl;

    connect(
        m_button, &QPushButton::pressed,
        this, [this]{
            m_value.press_button();
//            cout << "Press: " << this->font().pointSize() << endl;
//            cout << "height = " << this->height() << endl;
        }
    );
    value.add_listener(*this);
}


void ButtonOptionWidget::update_value(){
    m_label->setText(QString::fromStdString(m_value.label()));
    m_button->setEnabled(m_value.is_enabled());
    m_button->setText(QString::fromStdString(m_value.text()));

    int button_size = m_value.button_height();
    if (button_size > 0){
        m_button->setFixedHeight(button_size);
    }

    int text_size = m_value.text_size();
    if (text_size > 0){
        QFont font = m_button->font();
        font.setPointSize(text_size);
//        font.setPointSize(16);
        m_button->setFont(font);
//        cout << "Post set: " << this->font().pointSize() << endl;
    }
//    cout << "Update: " << this->font().pointSize() << endl;
}
void ButtonOptionWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}






}
