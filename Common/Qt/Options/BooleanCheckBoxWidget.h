/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxWidget_H
#define PokemonAutomation_BooleanCheckBoxWidget_H

#include <QCheckBox>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{



class BooleanCheckBoxCellWidget : public QCheckBox, public ConfigWidget, private ConfigOption::Listener{
public:
    ~BooleanCheckBoxCellWidget();
    BooleanCheckBoxCellWidget(QWidget& parent, BooleanCheckBoxCell& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    BooleanCheckBoxCell& m_value;
};




class BooleanCheckBoxOptionWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~BooleanCheckBoxOptionWidget();
    BooleanCheckBoxOptionWidget(QWidget& parent, BooleanCheckBoxOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    BooleanCheckBoxOption& m_value;
    QCheckBox* m_box;
};


}
#endif
