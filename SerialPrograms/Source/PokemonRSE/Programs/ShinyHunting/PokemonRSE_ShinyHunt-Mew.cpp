/*  E Shiny Mew
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_ShinyHunt-Mew.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

ShinyHuntMew_Descriptor::ShinyHuntMew_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:ShinyHuntMew",
        Pokemon::STRING_POKEMON + " RSE", "Shiny Hunt - Mew",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/ShinyHuntMew.md",
        "Use the Run Away method to shiny hunt Mew in Emerald.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct ShinyHuntMew_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntMew_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ShinyHuntMew::ShinyHuntMew()
    : NOTIFICATION_SHINY(
        "Shiny Found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MEW_WAIT_TIME(
        "<b>Mew wait time:</b><br>Wait this long after entering for Mew to hide in the grass.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , DOOR_TO_GRASS_TIME(
        "<b>Door to grass time:</b><br>Time it takes to run from the door to the edge of the tall grass. Three steps up.",
        LockMode::LOCK_WHILE_RUNNING,
        "400 ms"
    )
    , RIGHT_GRASS_1_TIME(
        "<b>First Right time:</b><br>Time it takes to turn right and take three steps. This follows the edge of the grass.",
        LockMode::LOCK_WHILE_RUNNING,
        "450 ms"
    )
    , UP_GRASS_1_TIME(
        "<b>Move Up time::</b><br>Time it takes turn up and take one step.",
        LockMode::LOCK_WHILE_RUNNING,
        "200 ms"
    )
    , RIGHT_GRASS_2_TIME(
        "<b>Second Right time:</b><br>Time it takes to turn right and take two steps.",
        LockMode::LOCK_WHILE_RUNNING,
        "260 ms"
    )
    , FACE_UP_TIME(
        "<b>Face Up time:</b><br>Time it takes to tap the up button and face up, without taking a step.",
        LockMode::LOCK_WHILE_RUNNING,
        "150 ms"
    )
{
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MEW_WAIT_TIME);
    PA_ADD_OPTION(DOOR_TO_GRASS_TIME);
    PA_ADD_OPTION(RIGHT_GRASS_1_TIME);
    PA_ADD_OPTION(UP_GRASS_1_TIME);
    PA_ADD_OPTION(RIGHT_GRASS_2_TIME);
    PA_ADD_OPTION(FACE_UP_TIME);
}

void ShinyHuntMew::enter_mew(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_UP, 250, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to enter area.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }
    else {
        env.log("Entered area.");
    }

    //Wait for Mew ! animation to finish
    pbf_wait(context, MEW_WAIT_TIME);
    context.wait_for_all_requests();

    //DO NOT pause while running!
    //Run up toward the extra tall grass - 3 steps
    ssf_press_button(context, BUTTON_B, 0ms, DOOR_TO_GRASS_TIME);
    pbf_press_dpad(context, DPAD_UP, DOOR_TO_GRASS_TIME, 0ms);

    //Turn right, take 3 steps
    ssf_press_button(context, BUTTON_B, 0ms, RIGHT_GRASS_1_TIME);
    pbf_press_dpad(context, DPAD_RIGHT, RIGHT_GRASS_1_TIME, 0ms);

    //Turn up, take 1 step
    ssf_press_button(context, BUTTON_B, 0ms, UP_GRASS_1_TIME);
    pbf_press_dpad(context, DPAD_UP, UP_GRASS_1_TIME, 0ms);

    //Turn right, take 2 steps
    ssf_press_button(context, BUTTON_B, 0ms, RIGHT_GRASS_2_TIME);
    pbf_press_dpad(context, DPAD_RIGHT, RIGHT_GRASS_2_TIME, 0ms);

    //Turn up. Start battle.
    pbf_press_dpad(context, DPAD_UP, FACE_UP_TIME, 0ms);

    context.wait_for_all_requests();
}

void ShinyHuntMew::exit_mew(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    ssf_press_button(context, BUTTON_B, 0ms, 400ms);
    pbf_press_dpad(context, DPAD_DOWN, 50, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 720ms);
    pbf_press_dpad(context, DPAD_LEFT, 90, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 800ms);
    pbf_press_dpad(context, DPAD_DOWN, 100, 20);

    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_DOWN, 250, 20);
            pbf_wait(context, 300);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }
    else {
        env.log("Exited area.");
    }
}

void ShinyHuntMew::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    ShinyHuntMew_Descriptor::Stats& stats = env.current_stats<ShinyHuntMew_Descriptor::Stats>();

    /*
    * Requires more precision to ensure a Mew encounter every time.
    * Movement is very configurable due to this.
    * Enter on the left side, run up and hug the grass to the edge just after the flower on the right
    * This but without the bike:
    * https://old.reddit.com/r/ShinyPokemon/comments/1c773oi/gen3_discuss_is_this_the_fastest_way_to_encounter/
    */

    while (true) {
        enter_mew(env, context);

        bool legendary_shiny = handle_encounter(env.console, context, true);
        if (legendary_shiny) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }
        env.log("No shiny found.");
        flee_battle(env.console, context);

        //Close dialog
        pbf_mash_button(context, BUTTON_B, 250);
        context.wait_for_all_requests();

        exit_mew(env, context);

        stats.resets++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
