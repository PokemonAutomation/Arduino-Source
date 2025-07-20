/*  Key Binding Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include "KeyBindingWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* KeyBindingCell::make_QtWidget(QWidget& parent){
    return new KeyBindingCellWidget(parent, *this);
}



KeyBindingCellWidget::~KeyBindingCellWidget(){
    m_value.remove_listener(*this);
}
KeyBindingCellWidget::KeyBindingCellWidget(QWidget& parent, KeyBindingCell& value)
    : QLineEdit(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    KeyBindingCellWidget::update_value();
    m_value.add_listener(*this);
}


void KeyBindingCellWidget::keyPressEvent(QKeyEvent* event){
    m_value.set(event->key());
}


void KeyBindingCellWidget::update_value(){
    QKeySequence seq((Qt::Key)(uint32_t)m_value);
//    cout << (uint32_t)m_value << endl;
    this->setText(seq.toString());
}
void KeyBindingCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}








}
