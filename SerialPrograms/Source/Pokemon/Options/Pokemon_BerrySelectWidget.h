/*  Pokemon Berry Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonBerrySelectWidget_H
#define PokemonAutomation_Pokemon_PokemonBerrySelectWidget_H

#include "Common/Qt/NoWheelComboBox.h"

namespace PokemonAutomation{
namespace Pokemon{


class BerrySelectWidget : public NoWheelComboBox{
public:
    BerrySelectWidget(
        QWidget& parent,
        const std::vector<std::string>& slugs,
        const std::string& current_slug
    );

    std::string slug() const;

private:
};


}
}
#endif
