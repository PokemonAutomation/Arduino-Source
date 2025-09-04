/*  Material Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_MaterialFarmer.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

MaterialFarmer_Descriptor::MaterialFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MaterialFarmer",
        STRING_POKEMON + " SV", "Material Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MaterialFarmer.md",
        "Farm materials - Happiny dust from Chanseys/Blisseys, for Item Printer.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

std::unique_ptr<StatsTracker> MaterialFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new MaterialFarmerStats());
}


MaterialFarmer::MaterialFarmer()
    : GO_HOME_WHEN_DONE(true)
    , MATERIAL_FARMER_OPTIONS(
        GroupOption::EnableMode::ALWAYS_ENABLED,
        nullptr,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATION_ERROR_FATAL
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void MaterialFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 50);

    // Throw user setup errors early in program
    // - Ensure language is set
    const Language language = MATERIAL_FARMER_OPTIONS.LANGUAGE;
    if (language == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game language option to read ingredient lists.");
    }

    // - Ensure audio input is enabled
    LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
    wait_until(
        env.console, context,
        std::chrono::milliseconds(1100),
        {audio_detector}
    );
    audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

    // start by warping to pokecenter for positioning reasons
    if (!MATERIAL_FARMER_OPTIONS.SKIP_WARP_TO_POKECENTER){
        reset_to_pokecenter(env.program_info(), env.console, context);
    }

    MaterialFarmerStats& stats = env.current_stats<MaterialFarmerStats>();

    try{
        run_material_farmer(env, env.console, context, MATERIAL_FARMER_OPTIONS, stats);
    }catch (ProgramFinishedException&){}

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
