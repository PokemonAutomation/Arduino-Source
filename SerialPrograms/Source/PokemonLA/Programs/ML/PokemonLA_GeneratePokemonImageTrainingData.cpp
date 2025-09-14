/*  Generate Pokemon Image Training Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_GeneratePokemonImageTrainingData.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


GeneratePokemonImageTrainingData_Descriptor::GeneratePokemonImageTrainingData_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:GeneratePokemonImageTrainingData",
        STRING_POKEMON + " LA", "Generate " + STRING_POKEMON + " Image Training Data",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/GeneratePokemonImageTrainingData.md",
        "Generate training images of " + STRING_POKEMON + " for machine learning purposes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


GeneratePokemonImageTrainingData::GeneratePokemonImageTrainingData()
    : NUM_BOXES(
        "<b>Number of Boxes to Process:</b><br>How many boxes to capture " + STRING_POKEMON + " from.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1, 30
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(NUM_BOXES);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GeneratePokemonImageTrainingData::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Send program starting notification
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    try{
        // Send completion notification
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    }catch(ProgramFinishedException&){
        // Program was stopped by user
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        throw;
    }catch(std::exception& e){
        // Handle any other errors
        send_program_fatal_error_notification(env, NOTIFICATION_ERROR_FATAL, "Fatal error occurred: " + std::string(e.what()));
        throw;
    }
}




}
}
}