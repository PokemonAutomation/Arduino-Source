/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
//#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "Common/Cpp/Exceptions.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV_SandwichMaker.h"

//#include <iostream>
//using std::cout;
//using std::endl;
#include <unordered_map>
#include <algorithm>

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
        PABotBaseLevel::PABOTBASE_12KB
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
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SandwichMaker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    #if 0
        // make unlimited sandwiches. until it errors out.
        while (true){
            make_sandwich_option(env, env.console, context, SANDWICH_OPTIONS);
            enter_sandwich_recipe_list(env.program_info(), env.console, context);
        }
    #endif

    make_sandwich_option(env, env.console, context, SANDWICH_OPTIONS);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

