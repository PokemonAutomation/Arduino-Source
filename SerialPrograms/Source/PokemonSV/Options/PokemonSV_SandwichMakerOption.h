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

class SandwichMakerOption : public GroupOption, private ConfigOption::Listener {

public:
    ~SandwichMakerOption();
    SandwichMakerOption();

    enum class HerbaSelection {
        sweet_herba_mystica,
        salty_herba_mystica,
        sour_herba_mystica,
        bitter_herba_mystica,
        spicy_herba_mystica,
    };
    const EnumDatabase<HerbaSelection>& HerbaSelection_Database();

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

    //Leave the herba out, the user selects them as HERBA_ONE and HERBA_TWO.
    //Remember the ingredient limits: Six fillings, four condiments. Two condiment slots will be taken by the Herba.
    const std::map<SandwichRecipe, std::vector<std::string>> PremadeSandwichIngredients{
        {SandwichRecipe::shiny_normal,  {"cucumber", "pickle", "tofu", "tofu", "tofu", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fire,    {"cucumber", "pickle", "red-bell-pepper", "red-bell-pepper","curry-powder","curry-powder", "red-bell-pepper"}},
        {SandwichRecipe::shiny_water,   {"cucumber", "pickle", "cucumber", "cucumber", "cucumber","curry-powder","curry-powder"}},
        {SandwichRecipe::shiny_electric,{"cucumber", "pickle", "yellow-bell-pepper", "yellow-bell-pepper", "yellow-bell-pepper", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_grass,   {"cucumber", "pickle", "lettuce", "lettuce", "lettuce", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ice,     {"cucumber", "pickle", "klawf-stick", "klawf-stick", "klawf-stick", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fighting,{"cucumber", "pickle", "pickle", "pickle", "pickle", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_poison,  {"cucumber", "pickle", "green-bell-pepper", "green-bell-pepper", "green-bell-pepper", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ground,  {"cucumber", "pickle", "ham", "ham", "ham", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_flying,  {"cucumber", "pickle", "prosciutto", "prosciutto", "prosciutto", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_psychic, {"cucumber", "pickle", "onion", "onion", "onion", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_bug,     {"cucumber", "pickle", "cherry-tomatoes", "cherry-tomatoes", "cherry-tomatoes", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_rock,    {"cucumber", "pickle", "bacon", "bacon", "bacon", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_ghost,   {"cucumber", "pickle", "red-onion", "red-onion", "red-onion", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_dragon,  {"cucumber", "pickle", "avocado", "avocado", "avocado", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_dark,    {"cucumber", "pickle", "smoked-fillet", "smoked-fillet", "smoked-fillet", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_steel,   {"cucumber", "pickle", "hamburger", "hamburger", "hamburger", "curry-powder", "curry-powder"}},
        {SandwichRecipe::shiny_fairy,   {"cucumber", "pickle", "tomato", "tomato", "tomato", "curry-powder", "curry-powder"}},
        {SandwichRecipe::huge_normal,   {"noodles", "noodles", "rice", "rice", "curry-powder", "curry-powder"}},
    };

    std::string herba_to_string(HerbaSelection value);
    std::vector<std::string> get_premade_ingredients(SandwichRecipe value);

    OCR::LanguageOCROption LANGUAGE;
    EnumDropdownOption<SandwichRecipe> SANDWICH_RECIPE;
    EnumDropdownOption<HerbaSelection> HERBA_ONE;
    EnumDropdownOption<HerbaSelection> HERBA_TWO;
    SandwichIngredientsTable SANDWICH_INGREDIENTS;

private:
    virtual void value_changed();

};

}
}
}
#endif
