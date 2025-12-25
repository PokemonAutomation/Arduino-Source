/*  Command Line Tool Main
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Command-line executable for Pokemon Automation utilities.
 *  GUI-free tool for tasks like camera stream checks, debugging, etc.
 */

#include <iostream>
#include "Common/Cpp/Color.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Logging/OutputRedirector.h"

using namespace PokemonAutomation;

int main(int argc, char* argv[]){
    // Set up output redirection for logging
    OutputRedirector redirect_stdout(std::cout, "stdout", Color());
    OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    // Get the global command-line logger (suitable for command-line tools)
    Logger& logger = global_logger_command_line();

    logger.log("================================================================================");
    logger.log("Starting Program...");
    logger.log("Pokemon Automation - Command Line Tool");

    if (argc > 1){
        logger.log("Arguments received:");
        for (int i = 1; i < argc; i++){
            logger.log(std::string("  [") + std::to_string(i) + "]: " + argv[i]);
        }
    }

    logger.log("Program completed successfully.");
    logger.log("================================================================================");

    return 0;
}
