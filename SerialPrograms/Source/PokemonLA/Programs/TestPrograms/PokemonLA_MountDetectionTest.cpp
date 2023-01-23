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
    : SingleSwitchProgramDescriptor(
        "PokemonLA:MountDetectionTest",
        STRING_POKEMON + " LA", "Mount Detection Test",
        "",
        "Test the mount detection in the bottom right corner.",
        FeedbackType::REQUIRED, AllowCommandsWhenRunning::ENABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MountDetectionTest::MountDetectionTest(){
    PA_ADD_OPTION(FAILED_ACTION);
}


void MountDetectionTest::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    MountTracker tracker(env.console, FAILED_ACTION);
    InferenceSession session(
        context, env.console,
        {{tracker, std::chrono::seconds(1)}}
    );
    context.wait_until_cancel();
}




}
}
}
