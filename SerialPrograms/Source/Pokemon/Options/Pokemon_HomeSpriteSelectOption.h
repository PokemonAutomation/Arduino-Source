/*  HomeSprite Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_HomeSpriteSelectOption_H
#define PokemonAutomation_PokemonBDSP_HomeSpriteSelectOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{



// Option to select a pokemon form with a Pokemon Home sprite shown.
// The forms are all the unique forms including shiny and non-shiny.
class HomeSpriteSelectCell : public StringSelectCell{
public:
    HomeSpriteSelectCell(const std::string& default_slug);
};




}
}
#endif
