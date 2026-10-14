/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ErrorReports/ProgramDumper.h"
#include "ComputerPrograms/UnitTestRunner.h"
#include "CommandLineTests.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



int run_command_line_tests(){
    //  Scratch CI debug: re-install the backtrace handler now that Qt has
    //  initialized (Qt installs its own fatal-signal handlers, which
    //  would otherwise win over the one set up in main()).
    setup_crash_handler();

    cout << "Running parallel unit tests..." << endl;
    ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
    if (runner.run()){
        return 1;
    }
    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
