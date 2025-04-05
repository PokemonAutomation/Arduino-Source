/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV_SandwichMaker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

SandwichMaker_Descriptor::SandwichMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:SandwichMaker",
        STRING_POKEMON + " SV", "Sandwich Maker",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/SandwichMaker.md",
        "Make a sandwich of your choice.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController}
    )
{}

SandwichMaker::SandwichMaker()
    : SANDWICH_OPTIONS(
        "Sandwich Options",
        nullptr,
        BaseRecipe::non_shiny,
        false,
        GroupOption::EnableMode::ALWAYS_ENABLED
    )
    , NUM_SANDWICHES(
        "<b>Number of sandwiches to make:</b><br>Repeatedly make the same sandwich.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 1, 1000
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(NUM_SANDWICHES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SandwichMaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    for (int i = 0; i < NUM_SANDWICHES; i++){
        env.console.log("Making sandwich number: " + std::to_string(i+1), COLOR_ORANGE);
        make_sandwich_option(env, env.console, context, SANDWICH_OPTIONS);
        enter_sandwich_recipe_list(env.program_info(), env.console, context);
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

