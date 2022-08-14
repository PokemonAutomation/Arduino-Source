/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TimeExpressionWidget_H
#define PokemonAutomation_Options_TimeExpressionWidget_H

#include <QWidget>
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "ConfigWidget.h"

class QLineEdit;

namespace PokemonAutomation{


template <typename Type>
class TimeExpressionWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~TimeExpressionWidget();
    TimeExpressionWidget(QWidget& parent, TimeExpressionOption<Type>& value);
    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override;

private:
    TimeExpressionOption<Type>& m_value;
    QLineEdit* m_box;
};


}
#endif
