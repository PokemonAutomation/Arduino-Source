/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameSelectWidget_H
#define PokemonAutomation_Pokemon_PokemonNameSelectWidget_H

#include "Common/Qt/NoWheelComboBox.h"

namespace PokemonAutomation{
namespace Pokemon{


class NameSelectWidget : public NoWheelComboBox{
public:
    NameSelectWidget(
        QWidget& parent,
        const std::map<std::string, QIcon>& icons,
        const std::vector<std::string>& slugs,
        const std::string& current_slug
    );

    std::string slug() const;
};



}
}
#endif
