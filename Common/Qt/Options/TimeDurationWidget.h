/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_TimeDurationWidget_H
#define PokemonAutomation_Options_TimeDurationWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



template <typename Type>
class TimeDurationCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~TimeDurationCellWidget();
    TimeDurationCellWidget(QWidget& parent, TimeDurationCell<Type>& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    TimeDurationCell<Type>& m_value;
};



template <typename Type>
class TimeDurationOptionWidget : public QWidget, public ConfigWidget{
public:
    ~TimeDurationOptionWidget();
    TimeDurationOptionWidget(QWidget& parent, TimeDurationOption<Type>& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    TimeDurationOption<Type>& m_value;
    QLineEdit* m_box;
};


}
#endif
