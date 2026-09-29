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
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_BlinkScenes.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_EyeBlinkDetector.h"
#include "PokemonBDSP_BedroomSeedFinder.h"
#include "PokemonBDSP_BlinkRecovery.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;



BedroomSeedFinder_Descriptor::BedroomSeedFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:BedroomSeedFinder",
        STRING_POKEMON + " BDSP", "Bedroom Seed Finder",
        "",
        "Recover the RNG seed by watching the player character's blinks in their bedroom.",
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
    std::vector<ImageRGB32> eyes = load_eye_templates(setups);

    std::vector<std::unique_ptr<EyeBlinkWatcher>> watchers;
    std::vector<PeriodicInferenceCallback> callbacks;
    make_blink_watchers(setups, eyes, watchers, callbacks);

    env.log(
        "Watching the player blink. The overlay box should be sitting on the player's eye."
    );

    BlinkRecoveryConfig config;
    config.npcs = 1;

    BlinkRecovery recovery = recover_state_from_blinks(
        env, context, watchers, callbacks, COLLECTION_DISPLAY, config
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
