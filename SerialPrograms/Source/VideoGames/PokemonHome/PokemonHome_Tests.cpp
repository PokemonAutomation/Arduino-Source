/*  Pokemon Home Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonHome_AlphaDetector.h"
#include "Inference/PokemonHome_BallReader.h"
#include "Inference/PokemonHome_BoxGenderDetector.h"
#include "Inference/PokemonHome_BoxViewDetector.h"
#include "Inference/PokemonHome_ButtonDetector.h"
#include "Inference/PokemonHome_GigantamaxDetector.h"
#include "Inference/PokemonHome_OriginMarkReader.h"
#include "Inference/PokemonHome_SelectionArrowDetector.h"
#include "Inference/PokemonHome_ShinyDetector.h"
#include "Inference/PokemonHome_SummaryReader.h"
#include "Inference/PokemonHome_SummaryScreenDetector.h"
#include "Inference/PokemonHome_TeraTypeReader.h"
#include "PokemonHome_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


void add_tests(UnitTestDatabase& database){
    add_tests_AlphaDetector(database);
    add_tests_BallReader(database);
    add_tests_BoxGenderDetector(database);
    add_tests_BoxViewDetector(database);
    add_tests_ButtonDetector(database);
    add_tests_GigantamaxDetector(database);
    add_tests_OriginMarkReader(database);
    add_tests_SelectionArrowDetector(database);
    add_tests_ShinyDetector(database);
    add_tests_SummaryReader(database);
    add_tests_SummaryScreenDetector(database);
    add_tests_TeraTypeReader(database);
}


}
}
}