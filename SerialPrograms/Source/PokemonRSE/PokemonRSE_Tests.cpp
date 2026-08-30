/*  Pokemon RSE Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "PokemonRSE/inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/inference/Menus/PokemonRSE_StartMenuDetector.h"
#include "PokemonRSE_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


void add_tests(UnitTestDatabase& database){
    add_tests_SelectionDialogDetector(database);
    add_tests_ConfirmSlotDetector(database);
    add_tests_StartMenuDetector(database);
    add_tests_StartMenuSlotDetector(database);
}


}
}
}