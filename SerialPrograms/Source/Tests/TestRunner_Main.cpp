/*  Unit Test Runner Main
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  SerialProgramsTests: an executable that runs the unit tests and nothing else. It runs the
 *  same tests as "SerialPrograms --command-line-test-mode" (see Tests/CommandLineTests.h), but
 *  opens no window, reads and writes no settings, and does not look for serial ports or cameras.
 *  So it can run while SerialPrograms is open on the same computer.
 *
 *  Usage:
 *      SerialProgramsTests [--filter <text>]... [--augment-color-distortions]
 *
 *  --filter <text>                 Select the tests whose names contain <text>. Repeat it to
 *                                  select the tests that match any of them. Default: all tests.
 *  --augment-color-distortions     Also run the image tests with color distortions.
 *
 *  Exit code: the number of failed tests (at most 254), or 255 for a bad argument or when no
 *  test matches the filters.
 *
 *  It finds Resources/ and the CommandLineTests repo the same way SerialPrograms does: in or
 *  above the folder that holds the executable.
 */

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <QCoreApplication>
#include "Common/Cpp/Concurrency/Qt6.9ThreadBugWorkaround.h"
#include "Common/Cpp/Concurrency/FireForgetDispatcher.h"
#include "Common/Cpp/Concurrency/PeriodicRunner.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Logging/FileLogger.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Logging/LastLogTracker.h"
#include "Common/Cpp/Logging/MultiOutputLogger.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "ComputerPrograms/ComputerPrograms_UnitTestRunner.h"
#include "Tests/CommandLineTests.h"

namespace PokemonAutomation{
namespace{

constexpr int EXIT_BAD_ARGUMENTS = 255;
constexpr size_t MAX_FAILURE_EXIT_CODE = 254;

struct TestRunnerArguments{
    //  Views of argv, which outlives every use.
    std::vector<std::string_view> filters;
    bool augment_color_distortions = false;
};

//  Return nothing if an argument is unknown or incomplete.
std::optional<TestRunnerArguments> parse_arguments(int argc, char* argv[]){
    TestRunnerArguments arguments;
    for (int i = 1; i < argc; i++){
        const std::string_view arg = argv[i];
        if (arg == "--filter" && i + 1 < argc){
            arguments.filters.emplace_back(argv[++i]);
        }else if (arg == "--augment-color-distortions"){
            arguments.augment_color_distortions = true;
        }else{
            std::cerr << "Unknown or incomplete argument: " << arg << "\n"
                << "Usage: SerialProgramsTests [--filter <text>]... [--augment-color-distortions]"
                << std::endl;
            return std::nullopt;
        }
    }
    return arguments;
}

//  The full test logs go to "SerialProgramsTests.log" in this executable's own user folder,
//  not in the folder of SerialPrograms.
const std::string& test_log_path(){
    static const std::string path =
        USER_FILE_PATH() + Filesystem::application_binary_name().stem().string() + ".log";
    return path;
}
FileLogger& test_file_logger(){
    static FileLogger logger(
        GlobalThreadPools::unlimited_normal(),
        FileLoggerConfig{
            .file_path = test_log_path(),
        }
    );
    return logger;
}

int run_tests(const TestRunnerArguments& arguments){
    std::vector<std::string> names = ComputerPrograms::unit_test_names(arguments.filters);
    if (names.empty()){
        std::cerr << "No test matches the filters." << std::endl;
        return EXIT_BAD_ARGUMENTS;
    }

    Logger& logger = global_logger_command_line();
    logger.log("Resources folder: " + RESOURCE_PATH());
    logger.log("Unit test resources folder: " + UNIT_TEST_RESOURCE_PATH());
    logger.log("Log file: " + test_log_path());

    size_t failed = run_unit_tests(arguments.augment_color_distortions, arguments.filters);
    return (int)std::min(failed, MAX_FAILURE_EXIT_CODE);
}

//  Same order as the end of main() in CommonFramework/Main.cpp, for the services the tests use.
void stop_global_services(){
    OCR::clear_ocr_cache();
    global_periodic_runner().stop();
    global_watchdog().stop();
    test_file_logger().stop();
}
void stop_thread_pools(){
    GlobalThreadPools::computation_realtime().stop();
    GlobalThreadPools::computation_normal().stop();
    GlobalThreadPools::unlimited_realtime().stop();
    GlobalThreadPools::unlimited_normal().stop();
    global_dispatcher.stop();

    GlobalThreadPools::qt_worker_threadpool().stop();
    GlobalThreadPools::qt_event_threadpool().stop();
}

//  The body of main(). `argc` is a reference because QCoreApplication keeps one.
int test_runner_main(int& argc, char* argv[]){
    std::optional<TestRunnerArguments> arguments = parse_arguments(argc, argv);
    if (!arguments){
        return EXIT_BAD_ARGUMENTS;
    }

    //  The tests use Qt (images, thread pools, events) but no window.
    QCoreApplication application(argc, argv);

    {
        MultiOutputLogger& logger = global_multi_logger();
        logger.add_listener(global_last_log_history());
        logger.add_listener(test_file_logger());
    }

    //  Construct the Qt thread pools on the main thread, as SerialPrograms does.
    GlobalThreadPools::qt_worker_threadpool();
    GlobalThreadPools::qt_event_threadpool();

    int ret = run_tests(*arguments);

    stop_global_services();
#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND
    //  See the Qt 6.9 workaround at the end of main() in CommonFramework/Main.cpp.
    std::cout.flush();
    std::quick_exit(ret);
#endif
    stop_thread_pools();
    return ret;
}

} // namespace


} // namespace PokemonAutomation


int main(int argc, char* argv[]){
    return PokemonAutomation::test_runner_main(argc, argv);
}
