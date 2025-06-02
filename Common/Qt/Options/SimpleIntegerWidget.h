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



template <typename Type>
class SimpleIntegerCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~SimpleIntegerCellWidget();
    SimpleIntegerCellWidget(QWidget& parent, SimpleIntegerCell<Type>& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerCell<Type>& m_value;
};



template <typename Type>
class SimpleIntegerOptionWidget : public QWidget, public ConfigWidget{
public:
    ~SimpleIntegerOptionWidget();
    SimpleIntegerOptionWidget(QWidget& parent, SimpleIntegerOption<Type>& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerCellWidget<Type>* m_cell;
};




}
#endif
