/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <float.h>
#include <chrono>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Async/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h"
#include "PokemonRSE_SoundListener.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{
    using namespace Pokemon;


SoundListener_Descriptor::SoundListener_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:SoundListener",
        STRING_POKEMON + " RSE", "Sound Listener",
        "",
        "Test sound detectors listening to audio stream.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}


SoundListener::SoundListener()
    : SOUND_TYPE("<b>Which Sound to Detect</b>",
        {
            {SoundType::SHINY, "shiny", "Shiny Sound"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SoundType::SHINY
    )
    , STOP_ON_DETECTED_SOUND(
        "<b>Stop on the detected sound</b><br>Stop program when the sound is detected.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(SOUND_TYPE);
    PA_ADD_OPTION(STOP_ON_DETECTED_SOUND);
}

void SoundListener::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // search_alpha_roar_from_audio_dump();
    // return;

    std::cout << "Running audio test program." << std::endl;

    std::shared_ptr<AudioInferenceCallback> detector;
    auto action = [&](float error_coefficient) -> bool{
        // This lambda function will be called when the sound is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_DETECTED_SOUND;
    };

    SoundType type = SOUND_TYPE;
    switch (type){
    case SoundType::SHINY:
        detector = std::make_shared<ShinySoundDetector>(env.console, action);
        break;
    default:
        throw InternalProgramError(
            &env.logger(), PA_CURRENT_FUNCTION,
            "Not such sound detector as sound type " + std::to_string((size_t)type)
        );
        return;
    }

    InferenceSession session(
        context, env.console,
        {{*detector, std::chrono::milliseconds(20)}}
    );
    context.wait_until_cancel();

    std::cout << "Audio test program Sound listener finished." << std::endl;
}



}
}
}
