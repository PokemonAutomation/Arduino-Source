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
        huge_fire,
        huge_water,
        huge_electric,
        huge_grass,
        huge_ice,
        huge_fighting,
        huge_poison,
        huge_ground,
        huge_flying,
        huge_psychic,
        huge_bug,
        huge_rock,
        huge_ghost,
        huge_dragon,
        huge_dark,
        huge_steel,
        huge_fairy,
        tiny_normal,
        tiny_fire,
        tiny_water,
        tiny_electric,
        tiny_grass,
        tiny_ice,
        tiny_fighting,
        tiny_poison,
        tiny_ground,
        tiny_flying,
        tiny_psychic,
        tiny_bug,
        tiny_rock,
        tiny_ghost,
        tiny_dragon,
        tiny_dark,
        tiny_steel,
        tiny_fairy,
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
        {SandwichRecipe::huge_normal,   {"tofu", "mustard", "mustard"}},
        {SandwichRecipe::huge_fire,     {"red-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_water,    {"cucumber", "mustard", "mustard"}},
        {SandwichRecipe::huge_electric, {"yellow-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_grass,    {"lettuce", "mustard", "mustard"}},
        {SandwichRecipe::huge_ice,      {"klawf-stick", "mustard", "mustard"}},
        {SandwichRecipe::huge_fighting, {"pickle", "mustard", "mustard"}},
        {SandwichRecipe::huge_poison,   {"green-bell-pepper", "mustard", "mustard"}},
        {SandwichRecipe::huge_ground,   {"ham", "mustard", "mustard"}},
        {SandwichRecipe::huge_flying,   {"prosciutto", "mustard", "mustard"}},
        {SandwichRecipe::huge_psychic,  {"onion", "mustard", "mustard"}},
        {SandwichRecipe::huge_bug,      {"cherry-tomatoes", "mustard", "mustard"}},
        {SandwichRecipe::huge_rock,     {"bacon", "mustard", "mustard"}},
        {SandwichRecipe::huge_ghost,    {"red-onion", "mustard", "mustard"}},
        {SandwichRecipe::huge_dragon,   {"avocado", "mustard", "mustard"}},
        {SandwichRecipe::huge_dark,     {"smoked-fillet", "mustard", "mustard"}},
        {SandwichRecipe::huge_steel,    {"hamburger", "mustard", "mustard"}},
        {SandwichRecipe::huge_fairy,    {"tomato", "mustard", "mustard"}},
        {SandwichRecipe::tiny_normal,   {"tofu", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fire,     {"red-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_water,    {"cucumber", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_electric, {"yellow-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_grass,    {"lettuce", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ice,      {"klawf-stick", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fighting, {"pickle", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_poison,   {"green-bell-pepper", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ground,   {"ham", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_flying,   {"prosciutto", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_psychic,  {"onion", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_bug,      {"cherry-tomatoes", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_rock,     {"bacon", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_ghost,    {"red-onion", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_dragon,   {"avocado", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_dark,     {"smoked-fillet", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_steel,    {"hamburger", "mayonnaise", "mayonnaise"}},
        {SandwichRecipe::tiny_fairy,    {"tomato", "mayonnaise", "mayonnaise"}},
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
