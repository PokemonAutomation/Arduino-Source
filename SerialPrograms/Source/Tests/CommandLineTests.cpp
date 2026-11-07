/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <iostream>
#include "CommonFramework/ErrorReports/CrashBacktrace.h"
#include "CommonFramework/Logging/Logger.h"
#include "ComputerPrograms/ComputerPrograms_UnitTestRunner.h"

#include "CommandLineTests.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{


int run_command_line_tests(bool augment_color_distortions){
    return run_unit_tests(augment_color_distortions, {}) == 0 ? 0 : 1;
}

size_t run_unit_tests(bool augment_color_distortions, std::span<const std::string_view> filters){
    install_crash_backtrace_handlers();
    //  Keep stdout line-buffered: when piped, stdio defaults to full
    //  buffering, and the last few KB of test output - including the lines
    //  right before a crash - would otherwise be lost in the buffer. An
    //  explicit buffer size is required: with 0, MSVC's ucrt can end up with
    //  a zero-size buffer, which corrupts the next stdout write.
    setvbuf(stdout, nullptr, _IOLBF, 8192);
    cout << "Running parallel unit tests..." << endl;
    ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
    size_t failed = runner.run(augment_color_distortions, filters);
    if (failed > 0){
        return failed;
    }
    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
