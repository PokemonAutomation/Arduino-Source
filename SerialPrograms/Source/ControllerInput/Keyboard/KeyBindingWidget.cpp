/*  Key Binding Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include "CommonFramework/Logging/Logger.h"
#include "ControllerInput/Keyboard/GlobalQtKeyMap.h"
#include "ControllerInput/Keyboard/KeyboardInput_KeyMappings.h"
#include "KeyBindingWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* KeyboardHidBindingCell::make_QtWidget(QWidget& parent){
    return new KeyboardHidBindingCellWidget(parent, *this);
}



KeyboardHidBindingCellWidget::~KeyboardHidBindingCellWidget(){
    m_value.remove_listener(*this);
}
KeyboardHidBindingCellWidget::KeyboardHidBindingCellWidget(QWidget& parent, KeyboardHidBindingCell& value)
    : QLineEdit(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    KeyboardHidBindingCellWidget::update_value();
    m_value.add_listener(*this);
}


void KeyboardHidBindingCellWidget::keyPressEvent(QKeyEvent* event){
    QtKeyMap::instance().record(*event);

    const std::map<QtKeyMap::QtKey, KeyboardKey>& MAP = get_keyid_to_hid_map();
    auto iter = MAP.find(QtKeyMap::QtKey(
        (Qt::Key)event->key(),
        (event->modifiers() & Qt::KeypadModifier) != 0
    ));

    if (iter == MAP.end()){
        global_logger_tagged().log("Unable to map Qt::Key to HID ID: " + std::to_string(event->key()));
        return;
    }

    m_value.set(iter->second);
}


void KeyboardHidBindingCellWidget::update_value(){
    const std::map<KeyboardKey, std::string>& MAP = KEYBOARDKEY_TO_STRING();
    auto iter = MAP.find(m_value);
    if (iter != MAP.end()){
        this->setText(QString::fromStdString(std::to_string((int)m_value) + ": " + iter->second));
    }else{
        this->setText(QString::fromStdString(std::to_string((int)m_value)));
    }
}
void KeyboardHidBindingCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}




}
