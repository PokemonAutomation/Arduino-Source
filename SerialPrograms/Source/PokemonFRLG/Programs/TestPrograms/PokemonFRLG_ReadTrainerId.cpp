/*  Read Trainer ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <string>
#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_TrainerCardDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_TrainerIdReader.h"
#include "PokemonFRLG_ReadTrainerId.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace std::chrono_literals;


ReadTrainerId_Descriptor::ReadTrainerId_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ReadTrainerId",
        Pokemon::STRING_POKEMON + " FRLG",
        "Read TID", "",
        "Read TID from the trainer card.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
){}

ReadTrainerId::ReadTrainerId()
{}

void ReadTrainerId::program(
    SingleSwitchProgramEnvironment &env,
    ProControllerContext &context
){
    env.log(
        "Starting Read Trainer ID program... Please ensure you are on the Trainer Card."
    );

    TrainerCardDetector detector(COLOR_RED);
    TrainerIdReader reader;
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    VideoSnapshot screen = env.console.video().snapshot();
    bool trainercard = detector.detect(screen);
    if (trainercard){
        env.log("Trainer Card detected.");
        env.log("Reading TID...");
        uint16_t tid = reader.read_tid(env.logger(), screen);
        env.log("TID: " + tid);
    }else{
        env.log("Trainer Card not detected!");
    }
    context.wait_for_all_requests();
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

