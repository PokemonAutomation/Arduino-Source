/*  Mount Detection Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA_MountDetectionTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


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


void MountDetectionTest::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    MountTracker tracker(env.console, (MountDetectorLogging)(size_t)FAILED_ACTION);
    InferenceSession session(
        context, env.console,
        {{tracker, std::chrono::seconds(1)}}
    );
    context.wait_until_cancel();
}




}
}
}
