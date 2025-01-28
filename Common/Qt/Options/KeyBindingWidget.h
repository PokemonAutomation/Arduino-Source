/*  Key Binding Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_KeyBindingWidget_H
#define PokemonAutomation_Options_KeyBindingWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/KeyBindingOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class KeyBindingCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~KeyBindingCellWidget();
    KeyBindingCellWidget(QWidget& parent, KeyBindingCell& value);

    virtual void update_value() override;
    virtual void value_changed(void* object) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    KeyBindingCell& m_value;
};




}
#endif
