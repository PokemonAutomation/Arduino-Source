/*  Donut Options Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h" //TODO: change/remove later
#include "PokemonLZA_DonutOptionsTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

DonutOptionsTest_Descriptor::DonutOptionsTest_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:DonutOptionsTest",
        STRING_POKEMON + " LZA", "Donut Options Test",
        "Programs/PokemonLZA/DonutOptionsTest.html",
        "Testing options layout for donut making.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

DonutOptionsTest::DonutOptionsTest()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(), //TODO: replace later or something
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BERRIES("<b>Berries:</b><br>The berries used to make the donut. Minimum 3 berries, maximum 8 berries.")
    , NUM_POWER_REQUIRED(
        "<b>Number of Powers to Match:</b><br>How many of a dount's powers must be in the the table below. Minimum 1, maximum 3. "
        "<br>Ex. For a target donut of Big Haul Lv.3, Berry Lv.3, and any or none for the 3rd power, set the number as 2."
        "<br>Then, in the flavor powers table, make sure to add Big Haul Lv.3 and Berry Lv. 3.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 3
        )
    , NUM_DONUTS(
        "<b>Number of Donuts:</b><br>The number of donuts to make.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BERRIES);
    PA_ADD_OPTION(NUM_POWER_REQUIRED);
    PA_ADD_OPTION(FLAVOR_POWERS);
    PA_ADD_OPTION(NUM_DONUTS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void DonutOptionsTest::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    const Language language = LANGUAGE;
    if (language == Language::None){
        throw UserSetupError(env.console, "Must set game language option to read ingredient lists.");
    }

    //Validate number of selected berries
    env.log("Checking berry count.");

    size_t num_berries = 0;
    std::vector<std::unique_ptr<DonutBerriesTableRow>> berries_table = BERRIES.copy_snapshot();
    num_berries = berries_table.size();

    if (num_berries < 3 || num_berries > 8) {
        throw UserSetupError(env.console, "Must have at least 3 berries and no more than 8 berries.");
    }
    env.log("Number of berries validated.", COLOR_BLACK);


    //Print table to log to check
    std::vector<std::unique_ptr<FlavorPowerTableRow>> wanted_powers_table = FLAVOR_POWERS.copy_snapshot();
    for (const std::unique_ptr<FlavorPowerTableRow>& row : wanted_powers_table){
        FlavorPowerTableEntry table_line = row->snapshot();
        env.log(table_line.to_str());
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

