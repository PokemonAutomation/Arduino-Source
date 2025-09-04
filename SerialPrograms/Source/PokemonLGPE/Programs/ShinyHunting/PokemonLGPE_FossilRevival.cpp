/*  LGPE Fossil Revival
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h"
#include "PokemonLGPE/Programs/PokemonLGPE_GameEntry.h"
#include "PokemonLGPE_FossilRevival.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

FossilRevival_Descriptor::FossilRevival_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:FossilRevival",
        Pokemon::STRING_POKEMON + " LGPE", "Fossil Revival",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/FossilRevival.md",
        "Shiny hunt fossil Pokemon by reviving and resetting.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct FossilRevival_Descriptor::Stats : public StatsTracker{
    Stats()
        : revives(m_stats["Revives"])
        , resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Revives");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& revives;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> FossilRevival_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

FossilRevival::FossilRevival()
    : SLOT(
        "<b>Fossil slot:</b><br>Position of the fossil in the selection dialog.",
        {
            {0, "slot0", "Slot 1"},
            {1, "slot1", "Slot 2"},
            {2, "slot2", "Slot 3"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    ,NUM_REVIVALS(
        "<b>Number of fossils to revive:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30, 1
    )
    , GO_HOME_WHEN_DONE(false)
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
    PA_ADD_OPTION(SLOT);
    PA_ADD_OPTION(NUM_REVIVALS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void FossilRevival::run_revives(SingleSwitchProgramEnvironment& env, JoyconContext& context){
    FossilRevival_Descriptor::Stats& stats = env.current_stats<FossilRevival_Descriptor::Stats>();
    //Press A to get to selection
    env.log("Starting dialog.");
    pbf_press_button(context, BUTTON_A, 100ms, 800ms);
    pbf_wait(context, 1000ms); //Wait for scientist to turn and face player
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    context.wait_for_all_requests();

    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //Select fossil slot
    env.log("Selecting fossil.");
    for (uint16_t c = 0; c < (uint16_t)SLOT.current_value(); c++){
        pbf_move_joystick(context, 128, 255, 100ms, 200ms);
    }
    context.wait_for_all_requests();

    //Mash A until revival over
    BlackScreenOverWatcher revival_over(COLOR_RED);
    int ret = run_until<JoyconContext>(
        env.console, context,
        [](JoyconContext& context){
            pbf_mash_button(context, BUTTON_A, 15000ms);
        },
        {revival_over}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        env.log("Failed to revive fossil.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to revive fossil.",
            env.console
        );
    }
    else {
        env.log("Fossil revived.");
    }

    //Now mash B until next black screen (end of summary/tucked pokemon away)
    BlackScreenOverWatcher summary_over(COLOR_YELLOW);
    int ret2 = run_until<JoyconContext>(
        env.console, context,
        [](JoyconContext& context){
            pbf_mash_button(context, BUTTON_B, 15000ms);
        },
        {summary_over}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        stats.errors++;
        env.update_stats();
        env.log("Did not detect summary over.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Did not detect summary over.",
            env.console
        );
    }
    else {
        env.log("Summary closed/Pokemon tucked away in box/team.");
    }

    //Close out come back soon text.
    pbf_mash_button(context, BUTTON_B, 1000ms);
    context.wait_for_all_requests();
}

void FossilRevival::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<RightJoycon>());
    assert_16_9_720p_min(env.logger(), env.console);
    FossilRevival_Descriptor::Stats& stats = env.current_stats<FossilRevival_Descriptor::Stats>();

    /*
    Settings:
    Text speed fast

    is first time dialog different? clear it out first.
    selection dialog still appears if only one fossil type

    Setup:
    Stand in front of the scientist in cinnebar.
    Save the game.
    Start the program in-game.
    */

    bool shiny_found = false;
    while (!shiny_found) {
        //Run revives
        for (uint16_t i = 0; i < NUM_REVIVALS; i++) {
            env.log("Running trade.");
            run_revives(env, context);

            stats.revives++;
            env.update_stats();
        }

        env.log("Done reviving. Checking boxes.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Done reviving. Checking boxes."
        );

        //Wait a bit.
        pbf_wait(context, 2500ms);
        context.wait_for_all_requests();

        //Open menu, open party, open boxes
        env.log("Opening boxes.");
        pbf_press_button(context, BUTTON_X, 200ms, 500ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1500ms);
        pbf_press_button(context, BUTTON_Y, 200ms, 2000ms);
        context.wait_for_all_requests();

        //Sort by order caught
        env.log("Sorting by order caught.");
        pbf_press_button(context, BUTTON_Y, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        context.wait_for_all_requests();

        //Press left to go to last (most recent) Pokemon
        env.log("Opening summary of most recent Pokemon.");
        pbf_move_joystick(context, 0, 128, 100ms, 100ms);
        context.wait_for_all_requests();

        //View summary - it takes a moment to load
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        context.wait_for_all_requests();

        pbf_wait(context, 5000ms);
        context.wait_for_all_requests();

        //Now check for shinies. Check everything that was traded.
        env.log("Checking received Pokemon.");
        for (uint16_t i = 0; i < NUM_REVIVALS; i++) {
            VideoSnapshot screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            bool check = shiny_checker.read(env.console.logger(), screen);

            if (check) {
                env.log("Shiny detected!");
                stats.shinies++;
                env.update_stats();
                send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", screen, true);
                shiny_found = true;

                //Back out to menu and favorite the shiny.
                env.log("Favoriting shiny.");
                pbf_press_button(context, BUTTON_B, 200ms, 5000ms);
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                pbf_move_joystick(context, 128, 0, 100ms, 100ms);
                pbf_move_joystick(context, 128, 0, 100ms, 100ms);
                pbf_move_joystick(context, 128, 0, 100ms, 200ms);
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
                pbf_press_button(context, BUTTON_B, 200ms, 800ms);

                //Go into summary again
                env.log("Navigating back into summary.");
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                pbf_move_joystick(context, 128, 255, 100ms, 100ms);
                pbf_move_joystick(context, 128, 255, 100ms, 100ms);
                pbf_press_button(context, BUTTON_A, 200ms, 100ms);
                context.wait_for_all_requests();
                pbf_wait(context, 5000ms);
                context.wait_for_all_requests();
            }
            else {
                env.log("Not shiny.");
            }

            //Move left, check next.
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_press_button(context, BUTTON_X, 0ms, 1000ms);
            context.wait_for_all_requests();
        }
        
        if (!shiny_found) {
            env.log("Out of Pokemon to check and no shiny found. Resetting game.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Out of Pokemon to check and no shiny found. Resetting game."
            );

            reset_game_from_game(env, env.console, context, &stats.errors, 3000ms);
            stats.resets++;
            env.update_stats();
        }
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
