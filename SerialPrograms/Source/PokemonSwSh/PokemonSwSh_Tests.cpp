/*  Pokemon Sword/Shield Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxShinySymbolDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



void add_tests(UnitTestDatabase& database){
    NintendoSwitch::PokemonSwSh::add_tests_BoxGenderDetector(database);
    NintendoSwitch::PokemonSwSh::add_tests_BoxShinySymbolDetector(database);
    NintendoSwitch::PokemonSwSh::add_tests_YCommDetector(database);
    NintendoSwitch::PokemonSwSh::add_tests_SelectionArrowFinder(database);
    NintendoSwitch::PokemonSwSh::MaxLairInternal::add_tests_MaxLairBattleMenuDetector(database);
}



}
}
}
