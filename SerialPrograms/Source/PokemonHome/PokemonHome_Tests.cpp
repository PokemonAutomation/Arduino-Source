/*  Pokemon Home Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonHome_BoxViewDetector.h"
#include "PokemonHome_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


void add_tests(UnitTestDatabase& database){
    add_tests_BoxViewDetector(database);
}


}
}
}