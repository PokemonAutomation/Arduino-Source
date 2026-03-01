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
// #include "CommonFramework/Logging/OutputRedirector.h"
#include "Integrations/PybindSwitchController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

using namespace PokemonAutomation;
using namespace PokemonAutomation::NintendoSwitch;

int main(int argc, char* argv[]){
//     // Set up output redirection for logging
//     OutputRedirector redirect_stdout(std::cout, "stdout", Color());
//     OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    // Get the global command-line logger (suitable for command-line tools)
    Logger& logger = global_logger_command_line();

    logger.log("================================================================================");
    logger.log("Starting Program...");
    logger.log("Pokemon Automation - Command Line Tool");

    // Check if port name argument is provided
    if (argc < 2){
        logger.log("Usage: " + std::string(argv[0]) + " <port_name>", COLOR_RED);
        logger.log("Example: " + std::string(argv[0]) + " cu.usbserial-0001");
        return 1;
    }

    // Test PybindSwitchProController
    logger.log("================================================================================");
    logger.log("Testing PybindSwitchProController...");

    try {
        // Create controller with port name from command-line argument
        const std::string port_name = argv[1];
        logger.log("Creating PybindSwitchProController with port: " + port_name);

        PybindSwitchProController controller(port_name);
        controller.wait_for_ready(5000);

        // Check if controller is ready
        if (controller.is_ready()){
            logger.log("Controller is ready!", COLOR_GREEN);
            logger.log("Status: " + controller.current_status());

            // Mash A button for 3 seconds
            logger.log("Mashing A button for 3 seconds...");

            for(int i = 0; i < 30; i++){
                // Push A button: delay=0ms, hold=50ms, release=50ms, button=BUTTON_A
                controller.push_button(0, 50, 50, static_cast<uint32_t>(BUTTON_A));
            }

            // Wait for all button commands to complete
            logger.log("Waiting for all requests to complete...");
            controller.wait_for_all_requests();

            logger.log("A button mashing completed!", COLOR_GREEN);
        }else{
            logger.log("Controller is not ready!", COLOR_RED);
            logger.log("Status: " + controller.current_status());
        }

    } catch (const std::exception& e){
        logger.log("Error during controller test: " + std::string(e.what()), COLOR_RED);
    }

    logger.log("================================================================================");
    logger.log("Program completed successfully.");
    logger.log("================================================================================");

    return 0;
}
