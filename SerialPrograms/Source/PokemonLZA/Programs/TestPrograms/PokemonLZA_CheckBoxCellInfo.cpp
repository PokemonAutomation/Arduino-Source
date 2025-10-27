/*  Check Box Cell Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxShinyDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxAlphaDetector.h"
#include "PokemonLZA_CheckBoxCellInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


CheckBoxCellInfo_Descriptor::CheckBoxCellInfo_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:CheckBoxCellInfo",
        STRING_POKEMON + " LZA", "Check Box Cell Info",
        "",
        "Test program to check inference on pokemon info in box system. "
        "When started, this program detects whether the cell is empty, and whether the Pokemon is shiny or alpha "
        "before immidiately finishes. Check the detection result on log overlay or in logs.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


CheckBoxCellInfo::CheckBoxCellInfo() {}


void CheckBoxCellInfo::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    env.log("Starting Check Box Cell Info test program...");

    // Take a snapshot of the current screen
    VideoSnapshot screen = env.console.video().snapshot();

    // Create detectors
    SomethingInBoxCellDetector cell_detector(COLOR_RED, &env.console.overlay());
    BoxShinyDetector shiny_detector(COLOR_BLUE, &env.console.overlay());
    BoxAlphaDetector alpha_detector(COLOR_PURPLE, &env.console.overlay());

    bool has_pokemon = cell_detector.detect(screen);
    bool is_shiny = shiny_detector.detect(screen);
    bool is_alpha = alpha_detector.detect(screen);

    // Summary - log to both text log and video overlay
    std::string pokemon_status = "Has Pokemon: " + std::string(has_pokemon ? "Yes" : "No");
    env.log(pokemon_status, COLOR_BLUE);
    env.console.overlay().add_log(pokemon_status, has_pokemon ? COLOR_GREEN : COLOR_ORANGE);

    std::string shiny_status = "Is Shiny: " + std::string(is_shiny ? "Yes" : "No");
    env.log(shiny_status, COLOR_BLUE);
    env.console.overlay().add_log(shiny_status, is_shiny ? COLOR_GREEN : COLOR_ORANGE);

    std::string alpha_status = "Is Alpha: " + std::string(is_alpha ? "Yes" : "No");
    env.log(alpha_status, COLOR_BLUE);
    env.console.overlay().add_log(alpha_status, is_alpha ? COLOR_GREEN : COLOR_ORANGE);

    env.log("Check Box Cell Info test program complete!", COLOR_GREEN);
    env.console.overlay().add_log("Detection Complete!", COLOR_GREEN);
}




}
}
}
