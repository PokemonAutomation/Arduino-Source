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



class BooleanCheckBoxCellWidget : public QWidget, public ConfigWidget{
public:
    ~BooleanCheckBoxCellWidget();
    BooleanCheckBoxCellWidget(QWidget& parent, BooleanCheckBoxCell& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    BooleanCheckBoxCell& m_value;
    QCheckBox* m_box;
};




class BooleanCheckBoxOptionWidget : public QWidget, public ConfigWidget{
public:
    ~BooleanCheckBoxOptionWidget();
    BooleanCheckBoxOptionWidget(QWidget& parent, BooleanCheckBoxOption& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    BooleanCheckBoxOption& m_value;
    QCheckBox* m_box;
};


}
#endif
