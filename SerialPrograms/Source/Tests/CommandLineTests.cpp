/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Logging/Logger.h"
#include "ComputerPrograms/ComputerPrograms_UnitTestRunner.h"
#include "CommandLineTests.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



int run_command_line_tests(bool augment_color_distortions){
    cout << "Running parallel unit tests..." << endl;
    ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
    if (runner.run(augment_color_distortions)){
        return 1;
    }
    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
