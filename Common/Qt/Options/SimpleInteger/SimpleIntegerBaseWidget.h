/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SimpleIntegerBaseWidget_H
#define PokemonAutomation_SimpleIntegerBaseWidget_H

#include <QLineEdit>
#include "SimpleIntegerBaseOption.h"

namespace PokemonAutomation{


template <typename Type>
class SimpleIntegerBaseWidget : public QWidget{
public:
    SimpleIntegerBaseWidget(QWidget& parent, SimpleIntegerBaseOption<Type>& value);
    void restore_defaults();

private:
    SimpleIntegerBaseOption<Type>& m_value;
    QLineEdit* m_box;
};



}
#endif
