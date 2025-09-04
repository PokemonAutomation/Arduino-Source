/*  Legendary Hunt - Emerald
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_LegendaryHunt-Emerald.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

LegendaryHuntEmerald_Descriptor::LegendaryHuntEmerald_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:LegendaryHuntEmerald",
        Pokemon::STRING_POKEMON + " RSE", "Legendary Hunt (Emerald)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/LegendaryHuntEmerald.md",
        "Use the Run Away method to shiny hunt legendaries in Emerald.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct LegendaryHuntEmerald_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> LegendaryHuntEmerald_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryHuntEmerald::LegendaryHuntEmerald()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::regis, "regis", "Regirock/Regice/Registeel"},
            {Target::groudon, "groudon", "Groudon"},
            {Target::kyogre, "kyogre", "Kyogre"},
            {Target::hooh, "hooh", "Ho-Oh"},
            {Target::lugia, "lugia", "Lugia"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::regis
    )
    , NOTIFICATION_SHINY(
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
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LegendaryHuntEmerald::reset_regi(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //turn around, walk down 4/until black screen over
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 960ms);
            pbf_press_dpad(context, DPAD_DOWN, 120, 20);
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
        env.log("Left area.");
    }
    pbf_wait(context, 50);
    context.wait_for_all_requests();

    //turn around, up one/black screen over
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_UP, 120, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
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

    //walk back up to the regi
    ssf_press_button(context, BUTTON_B, 0ms, 480ms);
    pbf_press_dpad(context, DPAD_UP, 60, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::reset_groudon(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //Turn left. Take 10 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 1440ms);
    pbf_press_dpad(context, DPAD_LEFT, 180, 20);

    //Turn up. Take 14 steps. (Bump into wall.)
    ssf_press_button(context, BUTTON_B, 0ms, 1920ms);
    pbf_press_dpad(context, DPAD_UP, 240, 20);

    //Turn right. Take 2 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 320ms);
    pbf_press_dpad(context, DPAD_RIGHT, 40, 20);

    //Turn up. Take 8 steps (Bump into wall.)
    ssf_press_button(context, BUTTON_B, 0ms, 1120ms);
    pbf_press_dpad(context, DPAD_UP, 140, 20);

    //Turn left. Take 4 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_LEFT, 80, 20);
    context.wait_for_all_requests();

    //Turn down. Exit. Black screen over.
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_DOWN, 90, 20);
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
        env.log("Left area.");
    }

    //Reverse above steps.
    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_UP, 90, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
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
    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1120ms);
    pbf_press_dpad(context, DPAD_DOWN, 140, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 320ms);
    pbf_press_dpad(context, DPAD_LEFT, 40, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1920ms);
    pbf_press_dpad(context, DPAD_DOWN, 240, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1440ms);
    pbf_press_dpad(context, DPAD_RIGHT, 180, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::reset_kyogre(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //Turn down. Take 1 step.
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);
    pbf_press_dpad(context, DPAD_DOWN, 20, 20);

    //Turn right. Take 9 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 1280ms);
    pbf_press_dpad(context, DPAD_RIGHT, 160, 20);

    //Turn up. 13 steps. Wall.
    ssf_press_button(context, BUTTON_B, 0ms, 1760ms);
    pbf_press_dpad(context, DPAD_UP, 220, 20);

    //Turn left. 4 steps. Wall.
    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_LEFT, 80, 20);

    //Turn up. 10 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 1440ms);
    pbf_press_dpad(context, DPAD_UP, 180, 20);

    //Turn right. 6 steps.
    ssf_press_button(context, BUTTON_B, 0ms, 880ms);
    pbf_press_dpad(context, DPAD_RIGHT, 110, 20);

    //Turn down. Exit. Black screen over.
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_DOWN, 90, 20);
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
        env.log("Left area.");
    }

    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_UP, 90, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
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

    ssf_press_button(context, BUTTON_B, 0ms, 880ms);
    pbf_press_dpad(context, DPAD_LEFT, 110, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1440ms);
    pbf_press_dpad(context, DPAD_DOWN, 180, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1760ms);
    pbf_press_dpad(context, DPAD_DOWN, 220, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1280ms);
    pbf_press_dpad(context, DPAD_LEFT, 160, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 160ms);
    pbf_press_dpad(context, DPAD_UP, 20, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::reset_hooh(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //Turn around, 10 steps down
    ssf_press_button(context, BUTTON_B, 0ms, 1440ms);
    pbf_press_dpad(context, DPAD_DOWN, 180, 20);

    //Turn right, take 1 step. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 240ms);
            pbf_press_dpad(context, DPAD_RIGHT, 30, 20);
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
        env.log("Left area.");
    }

    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //turn left, take one step. now turn back right and take a step. wait for black screen over.
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 320ms);
            pbf_press_dpad(context, DPAD_LEFT, 40, 20);

            ssf_press_button(context, BUTTON_B, 0ms, 320ms);
            pbf_press_dpad(context, DPAD_RIGHT, 40, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
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

    //reverse above steps, but only take 9 steps up
    //doesn't really matter since we want to trigger the encounter anyway
    ssf_press_button(context, BUTTON_B, 0ms, 240ms);
    pbf_press_dpad(context, DPAD_LEFT, 30, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 1360ms);
    pbf_press_dpad(context, DPAD_UP, 170, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::reset_lugia(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //Turn around, 5 steps down
    ssf_press_button(context, BUTTON_B, 0ms, 720ms);
    pbf_press_dpad(context, DPAD_DOWN, 90, 20);

    //Turn right, 3 steps right. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_RIGHT, 90, 20);
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
        env.log("Left area.");
    }

    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //turn up, take one step. then turn back down and take a step. wait for black screen over.
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 320ms);
            pbf_press_dpad(context, DPAD_UP, 40, 20);

            ssf_press_button(context, BUTTON_B, 0ms, 320ms);
            pbf_press_dpad(context, DPAD_DOWN, 40, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
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

    //reverse above steps
    ssf_press_button(context, BUTTON_B, 0ms, 560ms);
    pbf_press_dpad(context, DPAD_LEFT, 70, 20);

    ssf_press_button(context, BUTTON_B, 0ms, 720ms);
    pbf_press_dpad(context, DPAD_UP, 90, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    LegendaryHuntEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryHuntEmerald_Descriptor::Stats>();

    /*
    * Text speed fast, battle animations off
    * smoke ball or fast pokemon req. no entry effects.
    * 
    * Don't need to worry about PokeNav or random encounters for any of these targets.
    * 
    * Stand in front of Regis/Ho-Oh/Lugia. Save the game.
    */

    while (true) {
        switch (TARGET) {
        case Target::hooh:
        case Target::kyogre:
        case Target::groudon:
            //Step forward to start the encounter.
            pbf_press_dpad(context, DPAD_UP, 20, 50);
            break;
        //case Target::groudon: //Step up is easier.
        //    pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
        //    break;
        //case Target::kyogre:
        //    pbf_press_dpad(context, DPAD_LEFT, 20, 50);
        //    break;
        default:;
        }
        //handle_encounter presses A already for everything else
        
        bool legendary_shiny = handle_encounter(env.console, context, true);
        if (legendary_shiny) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }
        env.log("No shiny found.");
        flee_battle(env.console, context);
        
        //Close out dialog box
        pbf_mash_button(context, BUTTON_B, 250);
        context.wait_for_all_requests();
        
        //Exit and re-enter the room
        switch (TARGET) {
        case Target::regis:
            reset_regi(env, context);
            break;
        case Target::groudon:
            reset_groudon(env, context);
            break;
        case Target::kyogre:
            reset_kyogre(env, context);
            break;
        case Target::hooh:
            reset_hooh(env, context);
            break;
        case Target::lugia:
            reset_lugia(env, context);
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid target!",
                env.console
            );
            break;
        }

        stats.resets++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
