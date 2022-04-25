/*  Pokemon Ball Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonBallSelectWidget_H
#define PokemonAutomation_Pokemon_PokemonBallSelectWidget_H

#include "Common/Qt/NoWheelComboBox.h"

namespace PokemonAutomation{
namespace Pokemon{


class BallSelectWidget : public NoWheelComboBox{
public:
    BallSelectWidget(
        QWidget& parent,
        const std::vector<std::string>& slugs,
        const std::string& current_slug
    );

    std::string slug() const;
};


}
}
#endif
