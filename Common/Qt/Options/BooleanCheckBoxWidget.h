/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxWidget_H
#define PokemonAutomation_BooleanCheckBoxWidget_H

#include <QWidget>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

class QCheckBox;

namespace PokemonAutomation{


class BooleanCheckBoxWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~BooleanCheckBoxWidget();
    BooleanCheckBoxWidget(QWidget& parent, BooleanCheckBoxOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    BooleanCheckBoxOption& m_value;
    QCheckBox* m_box;
};


}
#endif
