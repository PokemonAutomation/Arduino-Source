/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <float.h>
#include <chrono>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_ShinySoundDetector.h"
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
        FeedbackType::REQUIRED, AllowCommandsWhenRunning::ENABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


SoundListener::SoundListener()
    : SOUND_TYPE("<b>Which Sound to Detect</b>",
        {
            {SoundType::Shiny,      "shiny",        "Shiny Sound"},
        },
        LockWhileRunning::LOCKED,
        SoundType::Shiny
    )
    , STOP_ON_DETECTED_SOUND(
        "<b>Stop on the detected sound</b><br>Stop program when the sound is detected.",
        LockWhileRunning::LOCKED,
        false
    )
{
    PA_ADD_OPTION(SOUND_TYPE);
    PA_ADD_OPTION(STOP_ON_DETECTED_SOUND);
}


// void searchAlphaRoarFromAudioDump();

void SoundListener::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    // pbf_move_right_joystick(context, 0, 255, 10, 0);

    // searchAlphaRoarFromAudioDump();
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
        detector = std::make_unique<ShinySoundDetector>(env.console.logger(), env.console, action);
        break;
    default:
        throw InternalProgramError(
            &env.logger(), PA_CURRENT_FUNCTION,
            "Not such sound detector as sound type " + std::to_string((size_t)type)
        );
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
