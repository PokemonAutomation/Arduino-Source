/*  BDSP Bedroom Seed Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_EyeBlinkDetector.h"
#include "PokemonBDSP_BedroomSeedFinder.h"
#include "PokemonBDSP_BlinkRecovery.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


const ImageFloatBox BEDROOM_PLAYER_EYE_BOX{0.4708, 0.4500, 0.0214, 0.0491};

const uint16_t GIVE_UP_SECONDS = 1500;


static std::vector<BdspEyeTemplate> bedroom_eye_templates(uint8_t player_model){
    if (player_model < 1 || player_model > BDSP_PLAYER_MODEL_COUNT){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unknown player model: " + std::to_string(player_model)
        );
    }
    return {
        BdspEyeTemplate{
            "bedroom_templates/model" + std::to_string(player_model) + ".png",
            BEDROOM_PLAYER_EYE_BOX,
            "Player"
        },
    };
}


BedroomSeedFinder_Descriptor::BedroomSeedFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:BedroomSeedFinder",
        STRING_POKEMON + " BDSP", "Bedroom Seed Finder",
        "",
        "Recover the RNG seed by watching the player blink in the bedroom.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


BedroomSeedFinder::BedroomSeedFinder()
    : GO_HOME_WHEN_DONE(false)
{
    PA_ADD_OPTION(PLAYER_MODEL);
    PA_ADD_OPTION(COLLECTION_DISPLAY);
    PA_ADD_OPTION(STATE_DISPLAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
}


void BedroomSeedFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    STATE_DISPLAY.reset();
    COLLECTION_DISPLAY.reset();

    std::vector<BdspEyeTemplate> setups = bedroom_eye_templates(PLAYER_MODEL.model_number());
    std::vector<std::shared_ptr<const ImageRGB32>> eyes = load_eye_templates(setups);

    std::vector<std::unique_ptr<EyeBlinkWatcher>> watchers;
    std::vector<PeriodicInferenceCallback> callbacks;
    make_blink_watchers(setups, eyes, watchers, callbacks);

    env.log(
        "Watching the player blink. Stand still in the bedroom and leave the camera alone. "
        "The overlay box should be sitting on the player's eye."
    );

    BlinkRecoveryConfig config;
    config.npcs = 1;

    BlinkRecovery recovery = recover_state_from_blinks(
        env, context, watchers, callbacks, COLLECTION_DISPLAY, config, GIVE_UP_SECONDS
    );
    if (!recovery.success){
        throw UserSetupError(env.logger(),
            "Gave up: " + recovery.failure_reason + ". Check the overlay box is on the "
            "player's eye, that the character model above matches the one in game, and that "
            "the log shows blinks arriving every few seconds."
        );
    }

    uint64_t seed0 = 0;
    uint64_t seed1 = 0;
    xorshift128_state_to_seed_pair(recovery.state, seed0, seed1);

    env.log("--------");
    env.log("State confirmed over " + std::to_string(recovery.events) + " rolls.", COLOR_BLUE);
    env.log("State: " + recovery.state.to_string(), COLOR_BLUE);
    env.log("PokeFinder seeds: " + tostr_hex_padded(16, seed0)
        + " " + tostr_hex_padded(16, seed1), COLOR_BLUE);

    STATE_DISPLAY.set_state(recovery.state, recovery.events);
    STATE_DISPLAY.set_confidence_unique();

    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
