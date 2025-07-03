/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Various functions to do sandwich making related automation.
 *  This file is used to share sandwich related code among different programs (egg auto, sandwich auto, etc.).
 */

#ifndef PokemonAutomation_PokemonSV_SandwichRoutines_H
#define PokemonAutomation_PokemonSV_SandwichRoutines_H

#include <cstddef>
#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"

namespace PokemonAutomation{
    
    struct ProgramInfo;
    class ProgramEnvironment;

namespace NintendoSwitch{
namespace PokemonSV{

// Assuming at picnic table, press A to start making sandwich.
// The function returns when the game shows the sandwich recipe menu.
// Return true if it enters the recipe menu. Return false if there is no ingredients on the plyaer character so
// no sandwich can be made.
bool enter_sandwich_recipe_list(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

// Starting at sandwich recipe selection menu, select the desired sandwich recipe and press A to enter
// the making sandwich mini game. It will use the first sandwich pick in the sandwich pick selection list.
// sandwich_index: [1, 151].
// Return true if it successfully finds and starts the recipe.
// Return false if the function fails to find the recipe. This could be that ingredients are not enough, and therefore
// the recipe cell is semi-transparent, failed to be detected.
bool select_sandwich_recipe(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t sandwich_index
);

// Assuming sandwich is made, press A repeatedly to finish eating animation until returning to picnic
void finish_sandwich_eating(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

// Assuming at sanwich recipe list, press X to enter custom sandwich mode
void enter_custom_sandwich_mode(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
);

enum class EggSandwichType{
    GREAT_PEANUT_BUTTER,
    TWO_SWEET_HERBS,
    BITTER_SWEET_HERBS,
    SALTY_SWEET_HERBS
};

// Assuming starting at the custom sandwich mode,
// select lettuce (assuming the first entry in the filling list), then select two herbs (two sweets, bitter sweet or salty sweet) in the end
// of the condiments list. The location of herbs are set by `xxx_herb_index_last`:
// if a herb is the last entry, set `xxx_herb_index_last` to 0;
// if a herb is second to last, set `xxx_herb_index_last` to 1; ...
// It will use the first sandwich pick in the sandwich pick selection list.
// After entering sandiwich mini game, it will drop the filling to quickly make a two-herb only sandwich to gain egg power lv 3.
void make_two_herbs_sandwich(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EggSandwichType sandwich_type,
    size_t sweet_herb_index_last,
    size_t salty_herb_index_last,
    size_t bitter_herb_index_last
);
// Assuming starting at the custom sandwich mode,
// select lettuce and two herbs (two sweets, bitter sweet or salty sweet according to `sandwich_type`).
// It will use the first sandwich pick in the sandwich pick selection list.
// After entering sandiwich mini game, it will drop the filling to quickly make a two-herb only sandwich to gain egg power lv 3.
void make_two_herbs_sandwich(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EggSandwichType sandwich_type,
    Language language
);

// Assuming starting at the sandwich recipe list,
// Process sandwich options and make a custom sandwich by calling make_sandwich_preset
void make_sandwich_option(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    SandwichMakerOption& SANDWICH_OPTIONS
);

// Assuming starting at the sandwich recipe list, make a sandwich given a preset ingredient/filling list
// calls run_sandwich_maker() when done
// ex. fillings = {{"apple", (uint8_t)1}}; condiments = {{"marmalade", (uint8_t)1}};
// make_sandwich_preset(env, context, language, fillings, condiments);
void make_sandwich_preset(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>& fillings,
    std::map<std::string, uint8_t>& condiments
);

// Assuming starting waiting for sandwich hand,
// Take a list of ingredients and make a sandwich
// Not meant to be run directly, use make_sandwich_option() or make_sandwich_preset() instead
// make great pb sandwich does call this directly, as it skips the custom sandwich menu
void run_sandwich_maker(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>& fillings,
    std::vector<std::string>& fillings_sorted,
    int& plates
);

}
}
}
#endif
