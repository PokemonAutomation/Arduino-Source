/*  Pokemon LZA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"
#include "Inference/Donuts/PokemonLZA_FlavorPowerScreenDetector.h"
#include "Inference/PokemonLZA_AlertEyeDetector.h"
#include "Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "Inference/PokemonLZA_ButtonDetector.h"
#include "Inference/Map/PokemonLZA_DirectionArrowDetector.h"
#include "Inference/PokemonLZA_DialogDetector.h"
#include "Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "Inference/PokemonLZA_MainMenuDetector.h"
#include "Inference/Map/PokemonLZA_MapDetector.h"
#include "Inference/Map/PokemonLZA_MapIconDetector.h"
#include "Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "Inference/PokemonLZA_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



void add_tests(UnitTestDatabase& database){
    add_tests_AlertEyeDetector(database);
    add_tests_BoxCellInfoDetector(database);
    add_tests_ButtonDetector(database);
    add_tests_DirectionArrowDetector(database);
    add_tests_DialogDetector(database);
    add_tests_DonutBerriesReader(database);
    add_tests_FlavorPowerScreenDetector(database);
    add_tests_HyperspaceCalorieDetector(database);
    add_tests_MainMenuDetector(database);
    add_tests_MapDetector(database);
    add_tests_MapIconDetector(database);
    add_tests_OverworldPartySelectionDetector(database);
    add_tests_SelectionArrowDetector(database);

}



}
}
}
