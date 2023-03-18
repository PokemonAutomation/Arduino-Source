/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to do sandwich making related automation.
 *  This file is used to share sandwich related code among different programs (egg auto, sandwich auto, etc.).
 */

#ifndef PokemonAutomation_PokemonSV_SandwichRoutines_H
#define PokemonAutomation_PokemonSV_SandwichRoutines_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include <cstddef>

namespace PokemonAutomation{
    
    struct ProgramInfo;
    class AsyncDispatcher;
    class ProgramEnvironment;
    class ConsoleHandle;
    class BotBaseContext;

namespace NintendoSwitch{
namespace PokemonSV{

void wait_for_initial_hand(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Assuming at picnic table, press A to start making sandwich.
// The function returns when the game shows the sandwich recipe menu.
// Return true if it enters the recipe menu. Return false if there is no ingredients on the plyaer character so
// no sandwich can be made.
bool enter_sandwich_recipe_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Starting at sandwich recipe selection menu, select the desired sandwich recipe and press A to enter
// the making sandwich mini game. It will use the first sandwich pick in the sandwich pick selection list.
// sandwich_index: [1, 151].
// Return true if it successfully finds and starts the recipe.
// Return false if the function fails to find the recipe. This could be that ingredients are not enough, and therefore
// the recipe cell is semi-transparent, failed to be detected.
bool select_sandwich_recipe(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, size_t sandwich_index);

ImageFloatBox expand_box(const ImageFloatBox& box);
ImageFloatBox hand_location_to_box(const std::pair<double, double>& loc);
std::string box_to_string(const ImageFloatBox& box);

ImageFloatBox move_sandwich_hand(
    const ProgramInfo& info,
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console,
    BotBaseContext& context,
    SandwichHandType hand_type,
    bool pressing_A,
    const ImageFloatBox& start_box,
    const ImageFloatBox& end_box
);

// Starting at the sandwich minigame of dropping ingredients, assume the selected recipe is Great Peanut Butter Sandwich,
// make the sandwich.
void build_great_peanut_butter_sandwich(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context);

// Assuming sandwich is made, press A repeatedly to finish eating animation until returning to picnic
void finish_sandwich_eating(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Assuming at sanwich recipe list, press X to enter custom sandwich mode
void enter_custom_sandwich_mode(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

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
    const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context,
    EggSandwichType sandwich_type, size_t sweet_herb_index_last, size_t salty_herb_index_last, size_t bitter_herb_index_last
);
// Assuming starting at the custom sandwich mode,
// select lettuce and two herbs (two sweets, bitter sweet or salty sweet according to `sandwich_type`).
// It will use the first sandwich pick in the sandwich pick selection list.
// After entering sandiwich mini game, it will drop the filling to quickly make a two-herb only sandwich to gain egg power lv 3.
void make_two_herbs_sandwich(
    const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context,
    EggSandwichType sandwich_type, Language language
);

}
}
}
#endif
