/*  Generate Pokemon Image Training Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/ScreenshotException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA_GeneratePokemonImageTrainingData.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;

const TravelLocation& to_travel_location(StartingLocation location){
    const TravelLocations& locations = TravelLocations::instance();

    switch(location){
    case StartingLocation::JUBILIFE_VILLAGE:
        break;
    case StartingLocation::FIELDLANDS_FIELDLANDS:
        return locations.Fieldlands_Fieldlands;
    case StartingLocation::FIELDLANDS_HEIGHTS:
        return locations.Fieldlands_Heights;
    case StartingLocation::MIRELANDS_MIRELANDS:
        return locations.Mirelands_Mirelands;
    case StartingLocation::MIRELANDS_BOGBOUND:
        return locations.Mirelands_Bogbound;
    case StartingLocation::COASTLANDS_BEACHSIDE:
        return locations.Coastlands_Beachside;
    case StartingLocation::COASTLANDS_COASTLANDS:
        return locations.Coastlands_Coastlands;
    case StartingLocation::HIGHLANDS_HIGHLANDS:
        return locations.Highlands_Highlands;
    case StartingLocation::HIGHLANDS_MOUNTAIN:
        return locations.Highlands_Mountain;
    case StartingLocation::HIGHLANDS_SUMMIT:
        return locations.Highlands_Summit;
    case StartingLocation::ICELANDS_SNOWFIELDS:
        return locations.Icelands_Snowfields;
    case StartingLocation::ICELANDS_ICEPEAK:
        return locations.Icelands_Icepeak;
    }

    // Should never reach here, but return default to avoid compiler warning
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid StartingLocation: " + std::to_string(int(location)));
}

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
    : STARTING_LOCATION(
        "<b>Starting Location:</b><br>Where you are starting the program.",
        {
            {StartingLocation::JUBILIFE_VILLAGE, "jubilife_village", "Jubilife Village"},
            {StartingLocation::FIELDLANDS_FIELDLANDS, "fieldlands_fieldlands", "Fieldlands - Fieldlands Camp"},
            {StartingLocation::FIELDLANDS_HEIGHTS, "fieldlands_heights", "Fieldlands - Heights Camp"},
            {StartingLocation::MIRELANDS_MIRELANDS, "mirelands_mirelands", "Mirelands - Mirelands Camp"},
            {StartingLocation::MIRELANDS_BOGBOUND, "mirelands_bogbound", "Mirelands - Bogbound Camp"},
            {StartingLocation::COASTLANDS_BEACHSIDE, "coastlands_beachside", "Coastlands - Beachside Camp"},
            {StartingLocation::COASTLANDS_COASTLANDS, "coastlands_coastlands", "Coastlands - Coastlands Camp"},
            {StartingLocation::HIGHLANDS_HIGHLANDS, "highlands_highlands", "Highlands - Highlands Camp"},
            {StartingLocation::HIGHLANDS_MOUNTAIN, "highlands_mountain", "Highlands - Mountain Camp"},
            {StartingLocation::HIGHLANDS_SUMMIT, "highlands_summit", "Highlands - Summit Camp"},
            {StartingLocation::ICELANDS_SNOWFIELDS, "icelands_snowfields", "Icelands - Snowfields Camp"},
            {StartingLocation::ICELANDS_ICEPEAK, "icelands_icepeak", "Icelands - Icepeak Camp"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartingLocation::JUBILIFE_VILLAGE
    )
    , NUM_BOXES(
        "<b>Number of Boxes to Process:</b><br>How many boxes to capture " + STRING_POKEMON + " from.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1, 30
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STARTING_LOCATION);
    PA_ADD_OPTION(NUM_BOXES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GeneratePokemonImageTrainingData::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Send program starting notification
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    try{
        // Step 1: Select starting location and navigate to box NPC
        select_starting_location(env, context);

        // Step 2: Talk to box NPC to access box system
        talk_to_box_npc(env, context);

        // TODO: Implement remaining steps
    }catch(ProgramFinishedException&){
        // Program was stopped by user
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        throw;
    }catch(ScreenshotException& e){
        std::string fail_message = e.message();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            fail_message,
            env.console
        );
        throw;
    }
    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

void GeneratePokemonImageTrainingData::select_starting_location(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Selecting starting location...");

    StartingLocation location = STARTING_LOCATION;
    const TravelLocation& travel_loc = to_travel_location(location);
    // fast travel to the target camp to reset player character position
    goto_any_camp_from_overworld(env, env.console, context, travel_loc);
        
    // move to the npc to open box system
    switch(location){
    case StartingLocation::JUBILIFE_VILLAGE:
        env.log("Starting at Jubilife Village");
        break;
    case StartingLocation::FIELDLANDS_FIELDLANDS:
        env.log("Starting at Fieldlands - Fieldlands Camp");
        pbf_move_left_joystick(context, 50, 0, 130, 10);
        break;
    case StartingLocation::FIELDLANDS_HEIGHTS:
        env.log("Starting at Fieldlands - Heights Camp");
        pbf_move_left_joystick(context, 40, 0, 165, 10);
        break;
    case StartingLocation::MIRELANDS_MIRELANDS:
        env.log("Starting at Mirelands - Mirelands Camp");
        pbf_move_left_joystick(context, 200, 0, 140, 10);
        break;
    case StartingLocation::MIRELANDS_BOGBOUND:
        env.log("Starting at Mirelands - Bogbound Camp");
        pbf_move_left_joystick(context, 0, 55, 215, 10);
        break;
    case StartingLocation::COASTLANDS_BEACHSIDE:
        env.log("Starting at Coastlands - Beachside Camp");
        pbf_move_left_joystick(context, 0, 35, 160, 10);
        break;
    case StartingLocation::COASTLANDS_COASTLANDS:
        env.log("Starting at Coastlands - Coastlands Camp");
        pbf_move_left_joystick(context, 0, 40, 180, 10);
        break;
    case StartingLocation::HIGHLANDS_HIGHLANDS:
        env.log("Starting at Highlands - Highlands Camp");
        pbf_move_left_joystick(context, 0, 20, 140, 10);
        break;
    case StartingLocation::HIGHLANDS_MOUNTAIN:
        env.log("Starting at Highlands - Mountain Camp");
        pbf_move_left_joystick(context, 0, 15, 100, 10);
        break;
    case StartingLocation::HIGHLANDS_SUMMIT:
        env.log("Starting at Highlands - Summit Camp");
        pbf_move_left_joystick(context, 10, 0, 150, 10);
        break;
    case StartingLocation::ICELANDS_SNOWFIELDS:
        env.log("Starting at Icelands - Snowfields Camp");
        pbf_move_left_joystick(context, 0, 20, 140, 10);
        break;
    case StartingLocation::ICELANDS_ICEPEAK:
        env.log("Starting at Icelands - Icepeak Camp");
        pbf_move_left_joystick(context, 20, 0, 180, 10);
        break;
    }
    context.wait_for_all_requests();

    // For now, assume player is already at the selected location
    // TODO: Add location-specific navigation logic if needed

    env.log("Location selection completed.");
}

void GeneratePokemonImageTrainingData::talk_to_box_npc(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Talking to box NPC...");
}




}
}
}