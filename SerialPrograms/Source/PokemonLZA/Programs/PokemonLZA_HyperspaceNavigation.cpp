/*  Hyperspace Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA_HyperspaceNavigation.h"


#include <atomic>
#include <format>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


bool check_calorie(
    ConsoleHandle& console,
    ProControllerContext& context,
    uint16_t min_calorie,
    uint16_t additional_calorie_buffer
){
    HyperspaceCalorieWatcher calorie_watcher(console.logger());
    int ret = wait_until(
        console, context, std::chrono::seconds(1), {calorie_watcher}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "check_calorie(): does not detect Calorie number after waiting for a second",
            console
        );
    }

    const uint16_t calorie_number = calorie_watcher.calorie_number();
    const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
    console.overlay().add_log(log_msg);
    console.log(log_msg);
    if (calorie_number <= min_calorie + additional_calorie_buffer){
        if (additional_calorie_buffer == 0){
            console.log("min calorie reached");
            console.overlay().add_log("Min Calorie Reached");
        } else{
            console.log("Close to min Calorie");
            console.overlay().add_log("Close to Min Calorie");
        }
        return true;
    }
    return false;
}

// Wait until the warp pad is detected
void detect_warp_pad(
    ConsoleHandle& console,
    ProControllerContext& context
){
    ButtonWatcher ButtonA(
        COLOR_RED,
        ButtonType::ButtonA,
        {0.4, 0.1, 0.2, 0.8},
        &console.overlay(),
        Milliseconds(100)
    );

    int ret = wait_until(
        console, context, std::chrono::seconds(5),
        {ButtonA}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "detect_warp_pad(): Cannot detect warp pad after 5 seconds",
            console
        );
    } else {
        console.log("Detected warp pad.");
    }
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
