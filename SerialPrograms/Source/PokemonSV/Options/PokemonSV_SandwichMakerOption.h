/*  Sandwich Makter Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichMakerOption_H
#define PokemonAutomation_PokemonSV_SandwichMakerOption_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "PokemonSV_SandwichIngredientsOption.h"
#include "PokemonSV_SandwichIngredientsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class SandwichMakerOption : public GroupOption {

public:
    SandwichMakerOption();

    enum class SandwichRecipe {
        shiny_normal,
        shiny_fire,
        shiny_water,
        shiny_electric,
        shiny_grass,
        shiny_ice,
        shiny_fighting,
        shiny_poison,
        shiny_ground,
        shiny_flying,
        shiny_psychic,
        shiny_bug,
        shiny_rock,
        shiny_ghost,
        shiny_dragon,
        shiny_dark,
        shiny_steel,
        shiny_fairy,
        huge_normal,
        custom,
    };

    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<SandwichRecipe> SANDWICH_RECIPE;
    SandwichIngredientsTable SANDWICH_INGREDIENTS;

};

}
}
}
#endif
