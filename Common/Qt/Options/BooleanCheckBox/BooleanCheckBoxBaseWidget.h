/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxBaseWidget_H
#define PokemonAutomation_BooleanCheckBoxBaseWidget_H

#include <QWidget>
#include "BooleanCheckBoxBaseOption.h"

class QCheckBox;

namespace PokemonAutomation{


class BooleanCheckBoxBaseWidget : public QWidget{
public:
    BooleanCheckBoxBaseWidget(QWidget& parent, BooleanCheckBoxBaseOption& value);
    void restore_defaults();

private:
    BooleanCheckBoxBaseOption& m_value;
    QCheckBox* m_box;
};



}
#endif
