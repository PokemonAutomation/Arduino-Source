/*  Pokemon BDSP Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_Tests_H
#define PokemonAutomation_PokemonBDSP_Tests_H

#include "Common/Cpp/TestRunners/UnitTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



// No controller context is accepted; this function only reads images and logs.
bool run_bag_screenshot_tests(Logger& logger, const std::string& folder);

void add_tests(UnitTestDatabase& database);



}
}
}
#endif
