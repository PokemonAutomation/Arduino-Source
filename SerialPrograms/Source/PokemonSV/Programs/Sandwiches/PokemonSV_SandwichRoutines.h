/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to do sandwich making related automation.
 *  This file is used to share sandwich related code among different programs (egg auto, sandwich auto, etc.).
 */

#ifndef PokemonAutomation_PokemonSV_SandwichRoutines_H
#define PokemonAutomation_PokemonSV_SandwichRoutines_H

namespace PokemonAutomation{
    
    class AsyncDispatcher;
    class ProgramEnvironment;
    class ConsoleHandle;
    class BotBaseContext;

namespace NintendoSwitch{
namespace PokemonSV{

// Assuming at picnic table, press A to start making sandwich.
// The function returns when the game shows the sandwich recipe menu.
// Return true if it enters the recipe menu. Return false if there is no ingredients on the plyaer character so
// no sandwich can be made.
// Will throw OperationFailedException if error occurs.
bool enter_sandwich_recipe_list(ConsoleHandle& console, BotBaseContext& context);

// Starting at sandwich recipe selection menu, select the desired sandwich recipe and press A to enter
// the making sandwich mini game. It will use the first sandwich pick in the sandwich pick selection list.
// sandwich_index: [1, 151].
// Return true if it successfully finds and starts the recipe.
// Return false if the function fails to find the recipe. This could be that ingredients are not enough, and therefore
// the recipe cell is semi-transparent, failed to be detected.
// Will throw OperationFailedException if error occurs.
bool select_sandwich_recipe(ConsoleHandle& console, BotBaseContext& context, size_t sandwich_index);

// Starting at the sandwich minigame of dropping ingredients, assume the selected recipe is Great Peanut Butter Sandwich,
// make the sandwich.
void make_great_peanut_butter_sandwich(AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context);

// Assuming sandwich is made, press A repeatedly to finish eating animation until returning to picnic
void finish_sandwich_eating(ConsoleHandle& console, BotBaseContext& context);

}
}
}
#endif
