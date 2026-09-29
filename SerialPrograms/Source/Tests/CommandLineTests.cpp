/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Logging/Logger.h"
#include "ComputerPrograms/UnitTestRunner.h"
#include "CommandLineTests.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



int run_command_line_tests(){
    cout << "Running parallel unit tests..." << endl;
    ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
    if (runner.run()){
        return 1;
    }
    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
