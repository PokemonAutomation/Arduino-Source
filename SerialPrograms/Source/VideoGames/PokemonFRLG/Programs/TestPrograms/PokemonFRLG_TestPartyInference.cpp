/*  Test Party Inference
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <string>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartyEmptySlotDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartyHeldItemDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG_TestPartyInference.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

TestPartyInference_Descriptor::TestPartyInference_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:TestPartyInference",
        Pokemon::STRING_POKEMON + " FRLG",
        "Test Party Inference", "",
        "Start on the party menu. Log party menu detection, empty slots, and held items from one snapshot.",
        PanelQualityStanding::DEVELOPER,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

TestPartyInference::TestPartyInference()
    : LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_OPTION(LANGUAGE);
}

void TestPartyInference::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    VideoOverlaySet overlays(env.console.overlay());
    PartyMenuDetector menu_detector(COLOR_RED);
    menu_detector.make_overlays(overlays);

    VideoSnapshot screen = env.console.video().snapshot();
    bool party_menu = menu_detector.detect(screen);
    env.log(std::string("PartyMenuDetector: ") + (party_menu ? "detected" : "not detected"));
    if (!party_menu){
        env.log("Party menu not detected. Slot results below may be invalid.", COLOR_RED);
    }

    const PartySlot slots[] = {
        PartySlot::ONE, PartySlot::TWO, PartySlot::THREE,
        PartySlot::FOUR, PartySlot::FIVE, PartySlot::SIX,
    };
    for (PartySlot slot : slots){
        scope.throw_if_cancelled();
        std::string empty_result = "N/A (slot 1 is always occupied)";
        if (slot != PartySlot::ONE){
            PartyEmptySlotDetector empty_detector(COLOR_CYAN, slot);
            empty_detector.make_overlays(overlays);
            empty_result = empty_detector.detect(screen) ? "yes" : "no";
        }

        PartyHeldItemDetector held_item_detector(COLOR_YELLOW, nullptr, slot, LANGUAGE);
        held_item_detector.make_overlays(overlays);
        bool held_item = held_item_detector.detect(screen);
        env.log(
            "Slot " + std::to_string(static_cast<unsigned>(slot) + 1) +
            ": PartyEmptySlotDetector = " + empty_result +
            "; PartyHeldItemDetector = " + (held_item ? "yes" : "no")
        );
    }

    env.log("Finished party inference. Detection regions remain visible for 10 seconds.", COLOR_BLUE);
    scope.wait_for(std::chrono::seconds(10));
}

}
}
}
