/*  Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/Inference/Menus/PokemonRSE_PartyMenuDetector.h"
#include "PokemonRSE/Inference/Menus/PokemonRSE_StartMenuDetector.h"
#include "PokemonRSE/Inference/Menus/PokemonRSE_SummaryDetector.h"
#include "PokemonRSE/Inference/PokemonRSE_ShinyNumberDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_GiftReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

GiftReset_Descriptor::GiftReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:GiftReset",
        Pokemon::STRING_POKEMON + " RSE", "Gift Reset",
        "Programs/PokemonRSE/GiftReset.html",
        "Soft reset for a shiny gift Pokemon.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct GiftReset_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> GiftReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

GiftReset::GiftReset()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::fossils,   "fossils",  "Lileep / Anorith"},
            {Target::castform,  "castform", "Castform"},
            {Target::beldum,    "beldum",   "Beldum"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::beldum
    )
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
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
        &NOTIFICATION_ERROR_RECOVERABLE,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GiftReset::obtain_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    GiftReset_Descriptor::Stats& stats = env.current_stats<GiftReset_Descriptor::Stats>();

    env.log("Obtaining Pokemon.");

    AdvanceDialogWatcher adv_white_start(COLOR_RED);
    int rets = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();
            }
        },
        { adv_white_start }
        );
    context.wait_for_all_requests();
    if (rets < 0){
        stats.errors++;
        env.update_stats();
        env.log("obtain_pokemon(): Unable to start starter dialog after 10 attempts.", COLOR_RED);
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "obtain_pokemon(): Unable to start starter dialog after 10 attempts.",
            env.console
        );
    }
    env.log("Initial A press completed.");

    bool seen_selection_arrow = false;

    while (true){
        context.wait_for_all_requests();

        AdvanceDialogWatcher adv_white(COLOR_RED);
        SelectionDialogWatcher selection_dialog(COLOR_RED);
        
        int ret = wait_until(
            env.console, context,
            10s,
            {
                adv_white,
                selection_dialog,
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
            if (!seen_selection_arrow){

                if (TARGET == Target::beldum){ //two yes/no boxes, castform and fossils???
                    env.log("First selection box detected. YES to obtain.");
                    seen_selection_arrow = true;
                    pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                }else{ //fossils
                    env.log("Selection box detected. NO to nickname.");
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                    context.wait_for_all_requests();
                    return;
                }
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);

            }else{
                env.log("Second selection box detected. NO to nickname.");
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                context.wait_for_all_requests();
                return;
            }
            continue;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "obtain_starter(): No recognized state after 10 seconds.",
                env.console
            );
        }
        //Small wait for dialog boxes
        //pbf_wait(context, 500ms);
        //context.wait_for_all_requests();
    }
    context.wait_for_all_requests();
}

void GiftReset::obtain_fossils(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Obtaining Fossil.");
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    //At least 6 lines of dialog/No to nickname for fossils
    //This takes care of the entire conversion+nickname+exit dialog
    int limit = 10;
    for (int i = 0; i < limit; i++){
        pbf_press_button(context, BUTTON_B, 320ms, 640ms);
        pbf_wait(context, 100ms);
        context.wait_for_all_requests();
    }
    context.wait_for_all_requests();
}

bool GiftReset::try_open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartMenuWatcher start_menu(COLOR_RED);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                pbf_wait(context, 100ms);
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_PLUS, 320ms, 640ms);
                pbf_wait(context, 100ms);
                context.wait_for_all_requests();
            }
        },
        { start_menu }
    );
    context.wait_for_all_requests();
    if (ret < 0){
        env.update_stats();
        env.log("open_summary(): Unable to open Start menu after 10 attempts.", COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "open_summary(): Unable to open Start menu after 10 attempts."
        );
        return false;
    }

    env.log("Navigating to party menu.");
    pbf_wait(context, 200ms);
    context.wait_for_all_requests();
    
    pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);

    /*
    if (!move_cursor_to_position(env.console, context, SelectionArrowPositionStartMenu::POKEMON)){
        std::string str = "open_summary(): Unable to move menu cursor to: " + Pokemon::STRING_POKEMON;
        env.log(str, COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            str
        );
        return false;
    }
    */

    //Open party menu
    PartyMenuWatcher blk1(COLOR_RED);

    int pm = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 320ms, 640ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { blk1 }
    );
    context.wait_for_all_requests();
    if (pm == 0){
        env.log("Entered party menu.");
    }else{
        env.log("open_summary(): Unable to enter party menu.", COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "open_summary(): Unable to enter party menu."
        );
        return false;
    }

    //Press up twice to get to the last slot
    PartySlotWatcher last_slot(COLOR_RED, PartySlot::SIX);
    int ps = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (int i = 0; i < 15; i++){ //Enough to cycle through 6pty+cxl twice
                pbf_wait(context, 320ms);
                context.wait_for_all_requests();
                pbf_press_dpad(context, DPAD_UP, 320ms, 320ms);
            }
        },
        { last_slot }
        );
    context.wait_for_all_requests();
    if (ps == 0){
        env.log("Moved selection to slot six.");
    }else{
        env.log("open_summary(): Unable to move selection to slot six.", COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "open_summary(): Unable to move selection to slot six."
        );
        return false;
    }

    //Two presses to open summary
    BlackScreenOverWatcher blk2(COLOR_RED);
    int sm = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
        },
        { blk2 }
    );
    if (sm == 0){
        env.log("Entered summary.");
    }else{
        env.log("open_summary(): Unable to enter summary.", COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "open_summary(): Unable to enter summary."
        );
        return false;
    }

    //Double check that we are on summary
    SummaryWatcher sum1(COLOR_RED);
    int sm1 = wait_until(
        env.console, context,
        std::chrono::seconds(5),
        {{ sum1 }}
    );
    if (sm1 == 0){
        env.log("Summary page dots detected.");
    }else{
        env.log("open_summary(): Unable to detect summary screen.", COLOR_RED);
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "open_summary(): Unable to detect summary screen."
        );
        return false;
    }

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    return true;
}
uint64_t GiftReset::open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    uint64_t errors = 0;
    for (; errors < 5; errors++){
        if (try_open_summary(env, context)){
            return errors;
        }else{
            env.log("Mashing B to return to overworld and retry...");
            pbf_mash_button(context, BUTTON_B, 10000ms);
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "open_summary(): Failed to open party summary after 5 attempts.",
        env.console
    );
}


void GiftReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //StartProgramChecks::check_performance_class_wired_or_wireless(context);

    GiftReset_Descriptor::Stats& stats = env.current_stats<GiftReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast.
    * Setup: 5 pokemon in your party. Stand in front of the pokemon. Save the game.
    * Move menu cursor back to the top (POKEDEX)
    */

    switch (TARGET){
    case Target::fossils:
        env.log("Target: Fossils");
        break;
    case Target::castform:
        env.log("Target: Castform");
        break;
    case Target::beldum:
        env.log("Target: Beldum");
        break;
    default:
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "GiftReset: Invalid target selection.",
            env.console
        );
        break;
    }

    bool shiny_found = false;

    while (!shiny_found){
        if (TARGET != Target::fossils && TARGET != Target::castform){
            obtain_pokemon(env, context);
        }else{
            obtain_fossils(env, context);
        }
        stats.errors += open_summary(env, context);

        VideoSnapshot screen = env.console.video().snapshot();

        ShinyNumberDetector shiny_checker(COLOR_YELLOW);
        shiny_found = shiny_checker.read(env.console.logger(), screen);

        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                screen,
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }else{
            env.log("Pokemon is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            stats.errors += soft_reset(env.console, context);
            stats.resets++;
            env.update_stats();
            context.wait_for_all_requests();
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

