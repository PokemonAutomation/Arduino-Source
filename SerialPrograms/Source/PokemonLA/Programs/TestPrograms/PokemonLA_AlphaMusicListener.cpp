/*  Alpha Music Listener
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
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_AlphaMusicDetector.h"
#include "PokemonLA_AlphaMusicListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

AlphaMusicListener_Descriptor::AlphaMusicListener_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:AlphaMusicListener",
        STRING_POKEMON + " LA", "Alpha Music Listener",
        "",
        "Detect alpha music from audio stream.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


AlphaMusicListener::AlphaMusicListener(const AlphaMusicListener_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , STOP_ON_ALPHA_MUSIC("<b>Stop on Alpha Music</b><br>Stop program when alpha music is heard.", false)
{
    PA_ADD_OPTION(STOP_ON_ALPHA_MUSIC);
}


void searchAlphaRoarFromAudioDump();

void AlphaMusicListener::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    // pbf_move_right_joystick(context, 0, 255, 10, 0);

    // searchAlphaRoarFromAudioDump();
    // return;

    std::cout << "Running audio test program." << std::endl;
    
    AlphaMusicDetector detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool{
        // This lambda function will be called when an alpha roar is detected.
        // Its return will determine whether to stop the program:
        return STOP_ON_ALPHA_MUSIC;
    });

    InferenceSession session(
        context, env.console,
        {{detector, std::chrono::milliseconds(20)}}
    );
    context.wait_until_cancel();


    std::cout << "Audio test program finished." << std::endl;
}



}
}
}
