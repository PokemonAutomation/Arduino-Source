/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "CommonTools/Async/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_ShinySoundDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



SoundListener_Descriptor::SoundListener_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:SoundListener",
        STRING_POKEMON + " LA", "Sound Listener",
        "",
        "Test sound detectors listening to audio stream.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


SoundListener::SoundListener()
    : SOUND_TYPE("<b>Which Sound to Detect</b>",
        {
            {SoundType::Shiny,          "shiny",        "Shiny Sound"},
            {SoundType::LetsGoKill,     "lets-go-kill", "Let's Go Kill"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SoundType::Shiny
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


// void search_alpha_roar_from_audio_dump();

void SoundListener::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  Connect the controller.
    // pbf_move_right_joystick(context, 0, 255, 10, 0);

    // search_alpha_roar_from_audio_dump();
    // return;

    std::cout << "Running audio test program." << std::endl;

    std::unique_ptr<AudioInferenceCallback> detector;
    auto action = [&](float error_coefficient) -> bool{
        // This lambda function will be called when the sound is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_DETECTED_SOUND;
    };

    SoundType type = SOUND_TYPE;
    switch (type){
    case SoundType::Shiny:
        detector = std::make_unique<ShinySoundDetector>(env.console, action);
        break;
    case SoundType::LetsGoKill:
        detector = std::make_unique<LetsGoKillSoundDetector>(env.console, action);
        break;
    default:
        throw InternalProgramError(
            &env.logger(), PA_CURRENT_FUNCTION,
            "Not such sound detector as sound type " + std::to_string((size_t)type)
        );
    }

#if 0
    ShinySoundDetector detector0(env.console, action);
    LetsGoKillSoundDetector detector1(env.console, action);
    InferenceSession session(
        context, env.console,
        {detector0, detector1}
    );
#else
    InferenceSession session(
        context, env.console,
        {*detector}
    );
#endif
    context.wait_until_cancel();

    std::cout << "Audio test program Sound listener finished." << std::endl;
}



}
}
}
