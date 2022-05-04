/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_TimeExpressionBaseWidget_H
#define PokemonAutomation_TimeExpressionBaseWidget_H

#include <QWidget>
#include <QLineEdit>
#include "TimeExpressionBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
class TimeExpressionBaseWidget : public QWidget{
public:
    TimeExpressionBaseWidget(QWidget& parent, TimeExpressionBaseOption<Type>& value);

    void restore_defaults();
    void update_ui();

private:
    TimeExpressionBaseOption<Type>& m_value;
    QLineEdit* m_box;
};



}
}
#endif
