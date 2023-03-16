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

    //TODO: Generic "herba" option that handles the exceptions on shiny chart v2
    const std::map<SandwichRecipe, std::vector<std::string>> PremadeSandwichIngredients{
        {SandwichRecipe::shiny_normal,  {"cucumber", "pickle", "tofu", "tofu", "tofu", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_fire,    {"cucumber", "pickle", "red-bell-pepper", "red-bell-pepper", "red-bell-pepper", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_water,   {"cucumber", "pickle", "cucumber", "cucumber", "cucumber", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_electric,{"cucumber", "pickle", "yellow-bell-pepper", "yellow-bell-pepper", "yellow-bell-pepper", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_grass,   {"cucumber", "pickle", "lettuce", "lettuce", "lettuce", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_ice,     {"cucumber", "pickle", "klawf-stick", "klawf-stick", "klawf-stick", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_fighting,{"cucumber", "pickle", "pickle", "pickle", "pickle", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_poison,  {"cucumber", "pickle", "green-bell-pepper", "green-bell-pepper", "green-bell-pepper", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_ground,  {"cucumber", "pickle", "ham", "ham", "ham", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_flying,  {"cucumber", "pickle", "prosciutto", "prosciutto", "prosciutto", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_psychic, {"cucumber", "pickle", "onion", "onion", "onion", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_bug,     {"cucumber", "pickle", "cherry-tomatoes", "cherry-tomatoes", "cherry-tomatoes", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_rock,    {"cucumber", "pickle", "bacon", "bacon", "bacon", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_ghost,   {"cucumber", "pickle", "red-onion", "red-onion", "red-onion", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_dragon,  {"cucumber", "pickle", "avocado", "avocado", "avocado", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_dark,    {"cucumber", "pickle", "smoked-fillet", "smoked-fillet", "smoked-fillet", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_steel,   {"cucumber", "pickle", "hamburger", "hamburger", "hamburger", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::shiny_fairy,   {"cucumber", "pickle", "tomato", "tomato", "tomato", "salty-herba-mystica", "salty-herba-mystica"}},
        {SandwichRecipe::huge_normal,   {"noodles", "noodles", "rice", "rice", "spicy-herba-mystica", "spicy-herba-mystica"}},
    };

    std::vector<std::string> get_premade_ingredients(SandwichRecipe value);

    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<SandwichRecipe> SANDWICH_RECIPE;
    SandwichIngredientsTable SANDWICH_INGREDIENTS;

};

}
}
}
#endif
