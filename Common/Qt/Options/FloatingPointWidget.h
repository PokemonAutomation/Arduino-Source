/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FloatingPointWidget_H
#define PokemonAutomation_FloatingPointWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{


class FloatingPointCellWidget : public QLineEdit, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FloatingPointCellWidget();
    FloatingPointCellWidget(QWidget& parent, FloatingPointCell& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    FloatingPointCell& m_value;
};


class FloatingPointOptionWidget : public QWidget, public ConfigWidget{
public:
    FloatingPointOptionWidget(QWidget& parent, FloatingPointOption& value);

    virtual void update() override;

private:
    FloatingPointOption& m_value;
    FloatingPointCellWidget* m_cell;
};



}
#endif
