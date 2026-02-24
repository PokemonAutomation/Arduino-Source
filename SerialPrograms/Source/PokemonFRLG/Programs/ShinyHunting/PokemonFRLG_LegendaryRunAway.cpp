/*  Legendary Run Away
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_LegendaryRunAway.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LegendaryRunAway_Descriptor::LegendaryRunAway_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:LegendaryRunAway",
        Pokemon::STRING_POKEMON + " FRLG", "Legendary Run Away",
        "Programs/PokemonFRLG/LegendaryRunAway.html",
        "Shiny hunt legendary Pokemon using the run away method.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct LegendaryRunAway_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> LegendaryRunAway_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryRunAway::LegendaryRunAway()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::hooh, "hooh", "Ho-Oh"},
            {Target::lugia, "lugia", "Lugia"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::hooh
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
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
    , HOOH_UP_DOWN(
        "<b>Ho-Oh up/down time:</b><br>Time it takes to run up to Ho-Oh or down away to reset.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , HOOH_LEFT_RIGHT(
        "<b>Ho-Oh left/right time:</b><br>Time it takes when facing the same direction to take one step left or right.",
        LockMode::LOCK_WHILE_RUNNING,
        "240 ms"
    )
    , LUGIA_UP_DOWN(
        "<b>Lugia up/down:</b><br>Time it takes to run up to Lugia or down away to reset.",
        LockMode::LOCK_WHILE_RUNNING,
        "600 ms"
    )
    , LUGIA_LEFT_RIGHT(
        "<b>Lugia left time:</b><br>Time it takes walk three steps left after entering Lugia's room.",
        LockMode::LOCK_WHILE_RUNNING,
        "550 ms"
    )
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(HOOH_UP_DOWN);
    PA_ADD_OPTION(HOOH_LEFT_RIGHT);
    PA_ADD_OPTION(LUGIA_UP_DOWN);
    PA_ADD_OPTION(LUGIA_LEFT_RIGHT);
}

void LegendaryRunAway::reset_hooh(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //Turn around, 10 steps down
    ssf_press_button(context, BUTTON_B, 0ms, HOOH_UP_DOWN);
    pbf_press_dpad(context, DPAD_DOWN, HOOH_UP_DOWN, 160ms);

    //Turn right, take 1 step. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, HOOH_LEFT_RIGHT, 160ms);
            pbf_wait(context, 2400ms);
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
        [&](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_LEFT, HOOH_LEFT_RIGHT, 160ms);

            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 500ms, 160ms);
            pbf_wait(context, 2400ms);
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
    pbf_press_dpad(context, DPAD_LEFT, HOOH_LEFT_RIGHT, 160ms);

    ssf_press_button(context, BUTTON_B, 0ms, HOOH_UP_DOWN);
    pbf_press_dpad(context, DPAD_UP, HOOH_UP_DOWN, 160ms);

    context.wait_for_all_requests();
}

void LegendaryRunAway::reset_lugia(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //Turn around, 5 steps down
    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_UP_DOWN);
    pbf_press_dpad(context, DPAD_DOWN, LUGIA_UP_DOWN, 160ms);

    //Turn right, 3 steps right. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_RIGHT, 720ms, 160ms);
            pbf_wait(context, 2400ms);
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
            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_UP, 500ms, 160ms);
            context.wait_for_all_requests();
            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_DOWN, 500ms, 160ms);
            pbf_wait(context, 2400ms);
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
    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_LEFT_RIGHT);
    pbf_press_dpad(context, DPAD_LEFT, LUGIA_LEFT_RIGHT, 160ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_UP_DOWN);
    pbf_press_dpad(context, DPAD_UP, LUGIA_UP_DOWN, 160ms);
    context.wait_for_all_requests();
}

void LegendaryRunAway::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAway_Descriptor::Stats& stats = env.current_stats<LegendaryRunAway_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast. Default borders. Battle animations off. Audio required.
    * Smoke Ball or fast pokemon required.
    * Setup: Stand in front of target. Save the game.
    * Lead can be shiny, reset is before lead is even sent out.
    * This is the same as RSE resets.
    */

    while (true) {
        if (TARGET == Target::hooh) {
            //Step forward to start the encounter.
            pbf_press_dpad(context, DPAD_UP, 160ms, 400ms);
        } else {
            //Press A to start the encounter.
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
        }
        
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
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
        
        //Exit and re-enter the room
        switch (TARGET) {
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

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

