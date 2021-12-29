/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDateBaseWidget_H
#define PokemonAutomation_SwitchDateBaseWidget_H

#include <QDateEdit>
#include "SwitchDateBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class SwitchDateBaseWidget : public QWidget{
public:
    SwitchDateBaseWidget(QWidget& parent, SwitchDateBaseOption& value);
    void restore_defaults();

private:
    SwitchDateBaseOption& m_value;
    QDateEdit* m_date_edit;
};




}
}
#endif
