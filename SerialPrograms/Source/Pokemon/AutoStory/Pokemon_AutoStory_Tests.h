/*  AutoStory Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The unit tests of the pieces every AutoStory program shares.
 *  add_tests_AutoStory() is the one entry point the unit test runner calls;
 *  a new shared AutoStory suite registers there.
 *
 */

#ifndef PokemonAutomation_Pokemon_AutoStory_Tests_H
#define PokemonAutomation_Pokemon_AutoStory_Tests_H

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Options/Pokemon_StoryRangeOptions_Tests.h"
#include "Pokemon_AutoStorySelect_Tests.h"

namespace PokemonAutomation{
namespace Pokemon{


// Registers every shared AutoStory suite, for the unit test runner.
inline void add_tests_AutoStory(UnitTestDatabase& database){
    add_tests_AutoStorySelect(database);
    add_tests_StoryRangeOptions(database);
}



} // namespace Pokemon
} // namespace PokemonAutomation
#endif
