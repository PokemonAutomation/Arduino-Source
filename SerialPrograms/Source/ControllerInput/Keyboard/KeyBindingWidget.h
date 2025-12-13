/*  Key Binding Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_KeyBindingWidget_H
#define PokemonAutomation_Options_KeyBindingWidget_H

#include <QLineEdit>
#include "KeyBindingOption.h"
#include "Common/Qt/Options/ConfigWidget.h"

namespace PokemonAutomation{



class KeyBindingCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~KeyBindingCellWidget();
    KeyBindingCellWidget(QWidget& parent, KeyBindingCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    KeyBindingCell& m_value;
};





class KeyboardHidBindingCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~KeyboardHidBindingCellWidget();
    KeyboardHidBindingCellWidget(QWidget& parent, KeyboardHidBindingCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    KeyboardHidBindingCell& m_value;
};





}
#endif
