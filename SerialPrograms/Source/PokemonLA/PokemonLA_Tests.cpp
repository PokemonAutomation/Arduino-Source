/*  Pokemon LA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



void add_tests(UnitTestDatabase& database){
    add_tests_BattleMenuDetector(database);
}



}
}
}
