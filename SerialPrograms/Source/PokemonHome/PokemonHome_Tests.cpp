/*  Pokemon Home Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonHome_BoxViewDetector.h"
#include "Inference/PokemonHome_SummaryScreenDetector.h"
#include "PokemonHome_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


void add_tests(UnitTestDatabase& database){
    add_tests_BoxViewDetector(database);
    add_tests_SummaryScreenDetector(database);
}


}
}
}