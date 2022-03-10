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
    , NAVIGATION_TIMEOUT(
        "<b>Navigation Timeout:</b><br>Give up if flag is not reached after this many seconds.",
        180, 0
    )
{
    PA_ADD_OPTION(NAVIGATION_TIMEOUT);
    PA_ADD_OPTION(SHINY_DETECTED);
}


void FlagNavigationTest::program(SingleSwitchProgramEnvironment& env){

    FlagNavigationAir session(
        env, env.console,
        SHINY_DETECTED.stop_on_shiny(),
        std::chrono::seconds(NAVIGATION_TIMEOUT)
    );
    session.run_session();

}



}
}
}
