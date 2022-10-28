/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TimeExpressionWidget_H
#define PokemonAutomation_Options_TimeExpressionWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



template <typename Type>
class TimeExpressionCellWidget : public QLineEdit, public ConfigWidget{
public:
    ~TimeExpressionCellWidget();
    TimeExpressionCellWidget(QWidget& parent, TimeExpressionCell<Type>& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    TimeExpressionCell<Type>& m_value;
};



template <typename Type>
class TimeExpressionOptionWidget : public QWidget, public ConfigWidget{
public:
    ~TimeExpressionOptionWidget();
    TimeExpressionOptionWidget(QWidget& parent, TimeExpressionOption<Type>& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    TimeExpressionOption<Type>& m_value;
    QLineEdit* m_box;
};


}
#endif
