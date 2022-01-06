/*  IV Checker Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVCheckerWidget_H
#define PokemonAutomation_Pokemon_IVCheckerWidget_H

#include "Common/Qt/NoWheelComboBox.h"
#include "Pokemon/Pokemon_IVChecker.h"

namespace PokemonAutomation{
namespace Pokemon{


class IVCheckerFilterWidget : public NoWheelComboBox{
public:
    IVCheckerFilterWidget(QWidget& parent, IVCheckerFilter current);

    operator IVCheckerFilter() const;

private:
};



}
}
#endif
