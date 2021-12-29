/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EnumDropdownWidget_H
#define PokemonAutomation_EnumDropdownWidget_H

#include <QComboBox>
#include "EnumDropdownOption.h"

namespace PokemonAutomation{



class EnumDropdownWidget : public QWidget, public ConfigWidget{
    Q_OBJECT

public:
    EnumDropdownWidget(QWidget& parent, EnumDropdownOption& value);
    virtual void restore_defaults() override;

signals:
    void on_changed();

private:
    EnumDropdownOption& m_value;
    QComboBox* m_box;
};





}
#endif
