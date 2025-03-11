/*  Test Map
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_TestMap_H
#define PokemonAutomation_Tests_TestMap_H

#include <string>
#include <functional>

namespace PokemonAutomation{


// For each test object, the command line test framework calls its specific
// TestFunction with the string parameter as each test file.
// TestFunction returns 0 if the test succeeds, > 0 if fails, < 0 if the test file is skipped.
using TestFunction = std::function<int(const std::string& test_file_path)>;

// Return the test function given test space and test object name.
// Return nullptr if it cannot be found.
// See CommandLineTests.h for details on test space and test object.
TestFunction find_test_function(const std::string& test_space, const std::string& test_obj_name);

}

#endif