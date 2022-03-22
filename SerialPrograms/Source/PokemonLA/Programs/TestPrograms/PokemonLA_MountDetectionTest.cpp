/*  Mount Detection Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/InferenceInfra/VisualInferenceSession.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA_MountDetectionTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


MountDetectionTest_Descriptor::MountDetectionTest_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:MountDetectionTest",
        STRING_POKEMON + " LA", "Mount Detection Test",
        "",
        "Test the mount detection in the bottom right corner.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MountDetectionTest::MountDetectionTest(const MountDetectionTest_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , FAILED_ACTION(
        "<b>Detection Failed Action</b>",
        {
            "Do Nothing",
            "Log to output window.",
            "Log to output window and save to file.",
        },
        2
    )
{
    PA_ADD_OPTION(FAILED_ACTION);
}


void MountDetectionTest::program(SingleSwitchProgramEnvironment& env){

    MountTracker tracker(env.console, (MountDetectorLogging)(size_t)FAILED_ACTION);
    VisualInferenceSession session(
        env, env.console, env.console, env.console,
        std::chrono::seconds(1)
    );
    session += tracker;

    session.run();


}




}
}
}
