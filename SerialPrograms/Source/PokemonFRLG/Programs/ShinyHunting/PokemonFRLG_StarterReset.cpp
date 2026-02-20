/*  Starter Reset
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
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

StarterReset_Descriptor::StarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:StarterReset",
        Pokemon::STRING_POKEMON + " FRLG", "Starter Reset",
        "Programs/PokemonFRLG/StarterReset.html",
        "Soft reset for a shiny starter.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct StarterReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinystarter(m_stats["Shiny Starter"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Starter");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinystarter;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterReset::StarterReset()
    : GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY_STARTER(
        "Shiny Starter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY_STARTER,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

//Pick up starter, say no to nickname
void StarterReset::obtain_starter(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    Ah, STARTER is your choice... red adv arrow
    So you're claiming... YES/NO box
    rly quite energetic... no arrow
    player received the starter from... no arrow, jingle must complete
    nickname YES/NO box
    wait, then rival picks up... no arrow
    rival received the ... no arrow DONE
    */

    env.log("Obtaining starter.");
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    bool seen_selection_arrow = false;
    //bool seen_nickname_arrow = false;
    while (true){
        context.wait_for_all_requests();

        AdvanceWhiteDialogWatcher adv_white(COLOR_RED);
        SelectionDialogWatcher selection_dialog(COLOR_RED);
        //WhiteDialogWatcher white_dialog_no_arrow(COLOR_RED);
        
        int ret = wait_until(
            env.console, context,
            10s,
            {
                adv_white,
                selection_dialog,
                //white_dialog_no_arrow,
            }
        );
        context.wait_for(500ms);

        switch (ret){
        case 0:
            env.log("Detected Advance Dialog. Pressing B.");
            pbf_press_button(context, BUTTON_B, 320ms, 640ms);
            continue;
        case 1:
            env.log("Detected Selection Dialog. Pressing A.");
            if (!seen_selection_arrow) {
                env.log("First selection box detected. YES to starter.");
                seen_selection_arrow = true;
                pbf_press_button(context, BUTTON_A, 320ms, 640ms);

                //Skip past energetic and jingle
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);

            } else {
                env.log("Second selection box detected. NO to nickname.");
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                //seen_nickname_arrow = true;

                //Press B some to try and skip the rival's pickup
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                context.wait_for_all_requests();
                return;
            }
            continue;
        //case 2:
        //    if (seen_selection_arrow) {
        //        env.log("White dialog box detected. Pressing B.");
        //        pbf_press_button(context, BUTTON_B, 320ms, 640ms);
        //    }
        //    continue;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "obtain_starter(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
    context.wait_for_all_requests();
}

//After declining to nickname, clear rival pickup and open your starter's summary
void StarterReset::open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    //From no to nickname to overworld
    StartMenuWatcher start_menu(COLOR_RED);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i = 0; i < 5; i++) {
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_PLUS, 320ms, 640ms);
            }
        },
        { start_menu }
    );
    context.wait_for_all_requests();
    if (ret < 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_summary(): Unable to open Start menu.",
            env.console
        );
    }

    //No Pokedex yet, so 1 A press to open party menu
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    BlackScreenOverWatcher blk1(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret1 = wait_until(
        env.console, context,
        5s,
        {blk1}
    );
    if (ret1 == 0){
        env.log("Entered party menu.");
    }else{
        env.log("Timed out waiting to enter game.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_summary(): Unable to enter Party menu.",
            env.console
        );
    }
    context.wait_for_all_requests();

    //Two presses to open summary
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    BlackScreenOverWatcher blk2(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret2 = wait_until(
        env.console, context,
        5s,
        {blk2}
    );
    if (ret2 == 0){
        env.log("Entered summary.");
    }else{
        env.log("Timed out waiting to enter game.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_summary(): Unable to enter summary.",
            env.console
        );
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

}


void StarterReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //StartProgramChecks::check_performance_class_wired_or_wireless(context);

    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast. Default borders.
    * Setup: Stand in front of the starter you want. Save the game.
    */

    bool shiny_starter = false;

    while (!shiny_starter) {
        obtain_starter(env, context);
        open_summary(env, context);

        VideoSnapshot screen = env.console.video().snapshot();

        ShinySymbolDetector shiny_checker(COLOR_YELLOW);
        shiny_starter = shiny_checker.read(env.console.logger(), screen);

        if (shiny_starter) {
            env.log("Shiny starter detected!");
            stats.shinystarter++;
            send_program_status_notification(env, NOTIFICATION_SHINY_STARTER, "Shiny starter found!", screen, true);
            break;
        } else {
            env.log("Starter is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            soft_reset(env.program_info(), env.console, context);
            stats.resets++;
            context.wait_for_all_requests();
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

