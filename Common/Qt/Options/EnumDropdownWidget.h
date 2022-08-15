/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownWidget_H
#define PokemonAutomation_Options_EnumDropdownWidget_H

#include <QWidget>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "ConfigWidget.h"

class QComboBox;

namespace PokemonAutomation{



class EnumDropdownWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~EnumDropdownWidget();
    EnumDropdownWidget(QWidget& parent, EnumDropdownOption& value);

    virtual void update() override;
    virtual void value_changed() override;

protected:
    EnumDropdownOption& m_value;
    QComboBox* m_box;
};





}
#endif
