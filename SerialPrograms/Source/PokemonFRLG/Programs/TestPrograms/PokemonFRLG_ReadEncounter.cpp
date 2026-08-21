/*  Read Encounter
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
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG_ReadEncounter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace std::chrono_literals;


ReadEncounter_Descriptor::ReadEncounter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ReadEncounter",
        Pokemon::STRING_POKEMON + " FRLG",
        "Read Wild Encounter", "",
        "Read name and level of a wild encounter.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
){}

ReadEncounter::ReadEncounter()
    : LANGUAGE(
        "<b>Game Language:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING, true
    )
{
    PA_ADD_OPTION(LANGUAGE);
}

void ReadEncounter::program(
    SingleSwitchProgramEnvironment& env,
    CancellableScope& scope
){
    env.log(
        "Starting Read Encounter program..."
    );

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading name and level...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, {});

    env.log("Name: " + encounter.name);

    env.log("Finished reading encounter.", COLOR_BLUE);
    scope.wait_for(10s);
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

