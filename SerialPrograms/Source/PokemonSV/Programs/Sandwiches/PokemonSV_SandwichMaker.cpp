/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Cpp/Exceptions.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_SandwichMaker.h"

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
    : GO_HOME_WHEN_DONE(false)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SandwichMaker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    assert_16_9_720p_min(env.logger(), env.console);

    //Player must be on default sandwich menu
    enter_custom_sandwich_mode(env.program_info(), env.console, context);
    if (SANDWICH_OPTIONS.LANGUAGE == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game langauge option to read ingredient lists.");
    }

    //SANDWICH_OPTIONS.SANDWICH_INGREDIENTS.
    //use pla custom path as example?


    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

