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

std::string SandwichMakerOption::herba_to_string(HerbaSelection value) {
    switch (value) {
    case HerbaSelection::bitter_herba_mystica:
        return "bitter-herba-mystica";
    case HerbaSelection::salty_herba_mystica:
        return "salty-herba-mystica";
    case HerbaSelection::sour_herba_mystica:
        return "sour-herba-mystica";
    case HerbaSelection::spicy_herba_mystica:
        return "spicy-herba-mystica";
    case HerbaSelection::sweet_herba_mystica:
        return "sweet-herba-mystica";
    }
    return "ERROR";
}

std::vector<std::string> SandwichMakerOption::get_premade_ingredients(SandwichRecipe value) {
    auto iter = PremadeSandwichIngredients.find(value);
    return iter->second;
}

bool SandwichMakerOption::two_herba_required(BaseRecipe value) {
    if (value == BaseRecipe::shiny || value == BaseRecipe::huge || value == BaseRecipe::tiny) {
        return true;
    }
    return false;
}

SandwichMakerOption::SandwichRecipe SandwichMakerOption::get_premade_sandwich_recipe(BaseRecipe base, PokemonType type, ParadoxRecipe paradox) {
    if (base == BaseRecipe::paradox)
    {
        auto iter = PremadeSandwichOther.find(paradox);
        return iter->second;
    }
    else {
        auto iter = PremadeSandwichType.find(std::make_pair(base, type));
        return iter->second;
    }
}

SandwichMakerOption::~SandwichMakerOption() {
    BASE_RECIPE.remove_listener(*this);
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
        true
    )
    , BASE_RECIPE(
        "<b>Sandwich Recipe:</b><br>Select a recipe to make a sandwich with preset ingredients, or select Custom Sandwich to make a sandwich using the table below. "
        "Refer to the documentation for recipe ingredients and valid Herba Mystica combinations.",
        {
            {BaseRecipe::shiny,     "shiny",    "Sparkling + Title + Encounter"},
            {BaseRecipe::huge,      "huge",     "Sparkling + Title + Humungo"},
            {BaseRecipe::tiny,      "tiny",     "Sparkling + Title + Teensy"},
            {BaseRecipe::paradox,   "paradox",  "Title + Encounter + Humungo/Teensy: Paradox-specific"},
            {BaseRecipe::custom,    "custom",   "Custom Sandwich"},
        },
        LockWhileRunning::LOCKED,
        BaseRecipe::shiny
        )
    , TYPE(
        "",
        {
            {PokemonType::normal,   "normal",   "Normal"},
            {PokemonType::fire,     "fire",     "Fire"},
            {PokemonType::water,    "water",    "Water"},
            {PokemonType::electric, "electric", "Electric"},
            {PokemonType::grass,    "grass",    "Grass"},
            {PokemonType::ice,      "ice",      "Ice"},
            {PokemonType::fighting, "fighting", "Fighting"},
            {PokemonType::poison,   "poison",   "Poison"},
            {PokemonType::ground,   "ground",   "Ground"},
            {PokemonType::flying,   "flying",   "Flying"},
            {PokemonType::psychic,  "psychic",  "Psychic"},
            {PokemonType::bug,      "bug",      "Bug"},
            {PokemonType::rock,     "rock",     "Rock"},
            {PokemonType::ghost,    "ghost",    "Ghost"},
            {PokemonType::dragon,   "dragon",   "Dragon"},
            {PokemonType::dark,     "dark",     "Dark"},
            {PokemonType::steel,    "steel",    "Steel"},
            {PokemonType::fairy,    "fairy",    "Fairy"},
        },
        LockWhileRunning::LOCKED,
        PokemonType::normal
    )
    , PARADOX(
        "",
        {
            {ParadoxRecipe::humungo3_greattusk_1,     "humungo3_greattusk_1",     "Great Tusk - Humungo #1"},
            {ParadoxRecipe::humungo2_screamtail_1,    "humungo2_screamtail_1",    "Scream Tail - Humungo #1"},
            {ParadoxRecipe::humungo2_screamtail_2,    "humungo2_screamtail_2",    "Scream Tail - Humungo #2"},
            {ParadoxRecipe::humungo2_brutebonnet_1,   "humungo2_brutebonnet_1",   "Brute Bonnet - Humungo #1"},
            {ParadoxRecipe::humungo2_brutebonnet_2,   "humungo2_brutebonnet_2",   "Brute Bonnet - Humungo #2"},
            {ParadoxRecipe::humungo3_fluttermane_1,   "humungo3_fluttermane_1",   "Flutter Mane - Humungo #1"},
            {ParadoxRecipe::humungo3_fluttermane_2,   "humungo3_fluttermane_2",   "Flutter Mane - Humungo #2"},
            {ParadoxRecipe::humungo2_slitherwing_1,   "humungo2_slitherwing_1",   "Slither Wing - Humungo #1"},
            {ParadoxRecipe::humungo2_slitherwing_2,   "humungo2_slitherwing_2",   "Slither Wing - Humungo #2"},
            {ParadoxRecipe::humungo3_sandyshocks_1,   "humungo3_sandyshocks_1",   "Sandy Shocks - Humungo #1"},
            {ParadoxRecipe::humungo3_sandyshocks_2,   "humungo3_sandyshocks_2",   "Sandy Shocks - Humungo #2"},
            {ParadoxRecipe::humungo3_roaringmoon_1,   "humungo3_roaringmoon_1",   "Roaring Moon - Humungo #1"},
            {ParadoxRecipe::humungo3_roaringmoon_2,   "humungo3_roaringmoon_2",   "Roaring Moon - Humungo #2"},
            {ParadoxRecipe::humungo2_irontreads_1,    "humungo2_irontreads_1",    "Iron Treads - Humungo #1"},
            {ParadoxRecipe::humungo2_irontreads_2,    "humungo2_irontreads_2",    "Iron Treads - Humungo #2"},
            {ParadoxRecipe::humungo2_ironbundle_1,    "humungo2_ironbundle_1",    "Iron Bundle - Humungo #1"},
            {ParadoxRecipe::humungo2_ironbundle_2,    "humungo2_ironbundle_2",    "Iron Bundle - Humungo #2"},
            {ParadoxRecipe::humungo2_ironhands_1,     "humungo2_ironhands_1",     "Iron Hands - Humungo #1"},
            {ParadoxRecipe::humungo2_ironhands_2,     "humungo2_ironhands_2",     "Iron Hands - Humungo #2"},
            {ParadoxRecipe::humungo2_ironjugulis_1,   "humungo2_ironjugulis_1",   "Iron Jugulis - Humungo #1"},
            {ParadoxRecipe::humungo2_ironjugulis_2,   "humungo2_ironjugulis_2",   "Iron Jugulis - Humungo #2"},
            {ParadoxRecipe::humungo3_ironmoth_1,      "humungo3_ironmoth_1",      "Iron Moth - Humungo #1"},
            {ParadoxRecipe::humungo3_ironmoth_2,      "humungo3_ironmoth_2",      "Iron Moth - Humungo #2"},
            {ParadoxRecipe::humungo3_ironthorns_1,    "humungo3_ironthorns_1",    "Iron Thorns - Humungo #1"},
            {ParadoxRecipe::humungo3_ironthorns_2,    "humungo3_ironthorns_2",    "Iron Thorns - Humungo #2"},
            {ParadoxRecipe::humungo2_ironvaliant_1,   "humungo2_ironvaliant_1",   "Iron Valiant - Humungo #1"},
            {ParadoxRecipe::humungo2_ironvaliant_2,   "humungo2_ironvaliant_2",   "Iron Valiant - Humungo #2"},
            {ParadoxRecipe::teensy3_greattusk_1,      "teensy3_greattusk_1",      "Great Tusk - Teensy #1"},
            {ParadoxRecipe::teensy2_screamtail_1,     "teensy2_screamtail_1",     "Scream Tail - Teensy #1"},
            {ParadoxRecipe::teensy2_screamtail_2,     "teensy2_screamtail_2",     "Scream Tail - Teensy #2"},
            {ParadoxRecipe::teensy2_brutebonnet_1,    "teensy2_brutebonnet_1",    "Brute Bonnet - Teensy #1"},
            {ParadoxRecipe::teensy3_fluttermane_1,    "teensy3_fluttermane_1",    "Flutter Mane - Teensy #1"},
            {ParadoxRecipe::teensy2_sliterwing_1,     "teensy2_sliterwing_1",     "Slither Wing - Teensy #1"},
            {ParadoxRecipe::teensy2_sliterwing_2,     "teensy2_sliterwing_2",     "Slither Wing - Teensy #2"},
            {ParadoxRecipe::teensy3_sandyshocks_1,    "teensy3_sandyshocks_1",    "Sandy Shocks - Teensy #1"},
            {ParadoxRecipe::teensy3_sandyshocks_2,    "teensy3_sandyshocks_2",    "Sandy Shocks - Teensy #2"},
            {ParadoxRecipe::teensy3_roaringmoon_1,    "teensy3_roaringmoon_1",    "Roaring Moon - Teensy #1"},
            {ParadoxRecipe::teensy2_irontreads_1,     "teensy2_irontreads_1",     "Iron Treads - Teensy #1"},
            {ParadoxRecipe::teensy2_irontreads_2,     "teensy2_irontreads_2",     "Iron Treads - Teensy #2"},
            {ParadoxRecipe::teensy2_ironbundle_1,     "teensy2_ironbundle_1",     "Iron Bundle - Teensy #1"},
            {ParadoxRecipe::teensy2_ironbundle_2,     "teensy2_ironbundle_2",     "Iron Bundle - Teensy #2"},
            {ParadoxRecipe::teensy2_ironhands_1,      "teensy2_ironhands_1",      "Iron Hands - Teensy #1"},
            {ParadoxRecipe::teensy2_ironjugulis_1,    "teensy2_ironjugulis_1",    "Iron Jugulis - Teensy #1"},
            {ParadoxRecipe::teensy3_ironmoth_1,       "teensy3_ironmoth_1",       "Iron Moth - Teensy #1"},
            {ParadoxRecipe::teensy3_ironthorns_1,     "teensy3_ironthorns_1",     "Iron Thorns - Teensy #1"},
            {ParadoxRecipe::teensy2_ironvaliant_1,    "teensy2_ironvaliant_1",    "Iron Valiant - Teensy #1"},
            {ParadoxRecipe::teensy2_ironvaliant_2,    "teensy2_ironvaliant_2",    "Iron Vailant - Teensy #2"},
        },
        LockWhileRunning::LOCKED,
        ParadoxRecipe::humungo3_greattusk_1
    )
    , HERBA_ONE(
        "<b>Herba Mystica:</b><br>Select the Herba Mystica to use in the preset recipe. Keep in mind which herb combinations are valid for the selected recipe.",
        {
            {HerbaSelection::sweet_herba_mystica,   "sweet-herba-mystica",  "Sweet Herba Mystica"},
            {HerbaSelection::salty_herba_mystica,   "salty-herba-mystica",  "Salty Herba Mystica"},
            {HerbaSelection::sour_herba_mystica,    "sour-herba-mystica",   "Sour Herba Mystica"},
            {HerbaSelection::bitter_herba_mystica,  "bitter-herba-mystica", "Bitter Herba Mystica"},
            {HerbaSelection::spicy_herba_mystica,   "spicy-herba-mystica",  "Spicy Herba Mystica"},
        },
        LockWhileRunning::LOCKED,
        HerbaSelection::salty_herba_mystica
    )
    , HERBA_TWO(
        "",
        {
            {HerbaSelection::sweet_herba_mystica,   "sweet-herba-mystica",  "Sweet Herba Mystica"},
            {HerbaSelection::salty_herba_mystica,   "salty-herba-mystica",  "Salty Herba Mystica"},
            {HerbaSelection::sour_herba_mystica,    "sour-herba-mystica",   "Sour Herba Mystica"},
            {HerbaSelection::bitter_herba_mystica,  "bitter-herba-mystica", "Bitter Herba Mystica"},
            {HerbaSelection::spicy_herba_mystica,   "spicy-herba-mystica",  "Spicy Herba Mystica"},
        },
        LockWhileRunning::LOCKED,
        HerbaSelection::salty_herba_mystica
    )
    , SANDWICH_INGREDIENTS("<b>Custom Sandwich:</b><br>Make a sandwich from the selected ingredients.<br>You must have at least one ingredient and one condiment, and no more than six ingredients and four condiments.")
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BASE_RECIPE);
    PA_ADD_OPTION(TYPE);
    PA_ADD_OPTION(PARADOX);
    PA_ADD_OPTION(HERBA_ONE);
    PA_ADD_OPTION(HERBA_TWO);
    PA_ADD_OPTION(SANDWICH_INGREDIENTS);

    SandwichMakerOption::value_changed();
    BASE_RECIPE.add_listener(*this);

}

void SandwichMakerOption::value_changed() {
    if (BASE_RECIPE == BaseRecipe::custom) {
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::ENABLED);
        HERBA_ONE.set_visibility(ConfigOptionState::DISABLED);
        HERBA_TWO.set_visibility(ConfigOptionState::DISABLED);
        TYPE.set_visibility(ConfigOptionState::DISABLED); //to prevent the options moving around
        PARADOX.set_visibility(ConfigOptionState::HIDDEN);
    }
    else if (two_herba_required(BASE_RECIPE)) { //shiny, huge, tiny
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::DISABLED);
        HERBA_ONE.set_visibility(ConfigOptionState::ENABLED);
        HERBA_TWO.set_visibility(ConfigOptionState::ENABLED);
        TYPE.set_visibility(ConfigOptionState::ENABLED);
        PARADOX.set_visibility(ConfigOptionState::HIDDEN);
    }
    else { //other
        SANDWICH_INGREDIENTS.set_visibility(ConfigOptionState::DISABLED);
        HERBA_ONE.set_visibility(ConfigOptionState::DISABLED);
        HERBA_TWO.set_visibility(ConfigOptionState::DISABLED);
        TYPE.set_visibility(ConfigOptionState::HIDDEN);
        PARADOX.set_visibility(ConfigOptionState::ENABLED);
    }
}


}
}
}
