/*  Pokemon FRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


void add_tests(UnitTestDatabase& database){
    add_tests_AdvanceWhiteDialogDetector(database);
    add_tests_SelectionDialogDetector(database);
    add_tests_ShinySymbolDetector(database);
    add_tests_PrizeSelectDetector(database);
    add_tests_AdvanceBattleDialogDetector(database);
    add_tests_BattleMenuDetector(database);
}


}
}
}