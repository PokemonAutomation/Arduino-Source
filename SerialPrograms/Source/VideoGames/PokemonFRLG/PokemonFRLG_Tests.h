/*  Pokemon FRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_Tests_H
#define PokemonAutomation_PokemonFRLG_Tests_H

#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{



void add_tests(UnitTestDatabase& database);

//  Shared helper for the readers that produce several values from one image.
UnitTestResult check_against_golden_file(
    const std::string& image_path,
    const std::vector<std::string>& labels,
    const std::vector<std::string>& values
);



}
}
}
#endif