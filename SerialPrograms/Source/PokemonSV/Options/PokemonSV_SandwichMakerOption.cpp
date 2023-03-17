/*  Sandwich Maker Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV_SandwichMakerOption.h"
#include "PokemonSV_SandwichIngredientsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

std::vector<std::string> SandwichMakerOption::get_premade_ingredients(SandwichRecipe token) {
    auto iter = PremadeSandwichIngredients.find(token);
    return iter->second;
}

SandwichMakerOption::~SandwichMakerOption() {
    SANDWICH_RECIPE.remove_listener(*this);
}

SandwichMakerOption::SandwichMakerOption()
    : GroupOption(
        "Sandwich Maker",
        LockWhileRunning::LOCKED,
        false, true
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read ingredients.",
        IV_READER().languages(),
        LockWhileRunning::LOCKED,
        false
    )
    , SANDWICH_RECIPE(
        "<b>Sandwich Recipe:</b><br>Select a recipe to make a sandwich with preset ingredients, or select Custom Sandwich to make a sandwich using the table below.<br>"
        "Sparkling/Title/Encounter: Cucumber + Pickle + 3x Ingredient + 2x Herba Mystica<br>",
        //"Sparkling/Title/Humungo: ???",
        {
            {SandwichRecipe::shiny_normal,      "shiny_normal",     "Sparkling + Title + Encounter: Normal"},
            {SandwichRecipe::shiny_fire,        "shiny_fire",       "Sparkling + Title + Encounter: Fire"},
            {SandwichRecipe::shiny_water,       "shiny_water",      "Sparkling + Title + Encounter: Water"},
            {SandwichRecipe::shiny_electric,    "shiny_electric",   "Sparkling + Title + Encounter: Electric"},
            {SandwichRecipe::shiny_grass,       "shiny_grass",      "Sparkling + Title + Encounter: Grass"},
            {SandwichRecipe::shiny_ice,         "shiny_ice",        "Sparkling + Title + Encounter: Ice"},
            {SandwichRecipe::shiny_fighting,    "shiny_fighting",   "Sparkling + Title + Encounter: Fighting"},
            {SandwichRecipe::shiny_poison,      "shiny_poison",     "Sparkling + Title + Encounter: Poison"},
            {SandwichRecipe::shiny_ground,      "shiny_ground",     "Sparkling + Title + Encounter: Ground"},
            {SandwichRecipe::shiny_flying,      "shiny_flying",     "Sparkling + Title + Encounter: Flying"},
            {SandwichRecipe::shiny_psychic,     "shiny_psychic",    "Sparkling + Title + Encounter: Psychic"},
            {SandwichRecipe::shiny_bug,         "shiny_bug",        "Sparkling + Title + Encounter: Bug"},
            {SandwichRecipe::shiny_rock,        "shiny_rock",       "Sparkling + Title + Encounter: Rock"},
            {SandwichRecipe::shiny_ghost,       "shiny_ghost",      "Sparkling + Title + Encounter: Ghost"},
            {SandwichRecipe::shiny_dragon,      "shiny_dragon",     "Sparkling + Title + Encounter: Dragon"},
            {SandwichRecipe::shiny_dark,        "shiny_dark",       "Sparkling + Title + Encounter: Dark"},
            {SandwichRecipe::shiny_steel,       "shiny_steel",      "Sparkling + Title + Encounter: Steel"},
            {SandwichRecipe::shiny_fairy,       "shiny_fairy",      "Sparkling + Title + Encounter: Fairy"},
            {SandwichRecipe::huge_normal,       "huge_normal",      "Sparkling + Title + Humungo: Normal"},
            {SandwichRecipe::custom,            "custom",           "Custom Sandwich"},
        },
        LockWhileRunning::LOCKED,
        SandwichRecipe::shiny_normal
    )
    , SANDWICH_INGREDIENTS("<b>Custom Sandwich:</b><br>Make a sandwich from the selected ingredients.<br>You must have at least one ingredient and one condiment, and no more than six ingredients and four condiments.")
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SANDWICH_RECIPE);
    PA_ADD_OPTION(SANDWICH_INGREDIENTS);

    SandwichMakerOption::value_changed();
    SANDWICH_RECIPE.add_listener(*this);

}

void SandwichMakerOption::value_changed() {
    if (SANDWICH_RECIPE == SandwichRecipe::custom) {
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::ENABLED);
    }
    else {
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::DISABLED);
    }
}


}
}
}
