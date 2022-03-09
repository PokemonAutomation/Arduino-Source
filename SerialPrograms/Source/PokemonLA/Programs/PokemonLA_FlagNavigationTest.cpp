/*  Flag Navigation Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "PokemonLA_FlagNavigationAir.h"
#include "PokemonLA_FlagNavigationTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


FlagNavigationTest_Descriptor::FlagNavigationTest_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:FlagNavigationTest",
        STRING_POKEMON + " LA", "Flag Navigation Test",
        "",
        "Navigate to the flag pin.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


FlagNavigationTest::FlagNavigationTest(const FlagNavigationTest_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}


void FlagNavigationTest::program(SingleSwitchProgramEnvironment& env){

    FlagNavigationAir session(env, env.console, true);
    session.run_session();

}



}
}
}
