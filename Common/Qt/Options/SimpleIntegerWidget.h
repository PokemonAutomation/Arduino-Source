/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SimpleIntegerWidget_H
#define PokemonAutomation_SimpleIntegerWidget_H

#include <QLineEdit>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{



class SimpleIntegerCellWidget : public QLineEdit, public ConfigWidget{
public:
    using ParentOption = SimpleIntegerCellBase;
    using NativeType = ParentOption::NativeType;

public:
    ~SimpleIntegerCellWidget();
    SimpleIntegerCellWidget(QWidget& parent, SimpleIntegerCellBase& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerCellBase& m_value;
};



class SimpleIntegerOptionWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = SimpleIntegerOptionBase;
    using NativeType = ParentOption::NativeType;

public:
    ~SimpleIntegerOptionWidget();
    SimpleIntegerOptionWidget(QWidget& parent, SimpleIntegerOptionBase& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerCellWidget* m_cell;
};




}
#endif
