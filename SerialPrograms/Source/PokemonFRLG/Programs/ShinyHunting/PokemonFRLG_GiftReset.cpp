/*  Gift Reset
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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG_GiftReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

GiftReset_Descriptor::GiftReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:GiftReset",
        Pokemon::STRING_POKEMON + " FRLG", "Gift Reset",
        "Programs/PokemonFRLG/GiftReset.html",
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
            {Target::starters, "starters", "Bulbasaur / Squirtle / Charmander"},
            {Target::hitmon, "hitmon", "Magikarp, Hitmonlee / Hitmonchan"},
            {Target::eevee, "eevee", "Eevee"},
            {Target::lapras, "lapras", "Lapras"},
            {Target::fossils, "fossils", "Omanyte / Kabuto / Aerodactyl "},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::starters
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

//Pick up starter, say no to nickname
void GiftReset::obtain_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    GiftReset_Descriptor::Stats& stats = env.current_stats<GiftReset_Descriptor::Stats>();

    /*
    Ah, starter is your choice... red adv arrow
    So you're claiming... YES/NO box
    rly quite energetic... no arrow
    player received the starter from... no arrow, jingle must complete
    nickname YES/NO box
    wait, then rival picks up... no arrow
    rival received the ... no arrow DONE
    */

    env.log("Obtaining Pokemon.");
    if (TARGET == Target::starters){
        AdvanceWhiteDialogWatcher adv_white_start(COLOR_RED);
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
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "obtain_pokemon(): Unable to start starter dialog after 10 attempts.",
                env.console
            );
        }
        env.log("Initial A press completed.");
    }else{
        //Need to double check what the first dialog box is for the other gifts
        pbf_press_button(context, BUTTON_A, 320ms, 640ms);
    }
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
            if (!seen_selection_arrow){

                if (TARGET == Target::starters || TARGET == Target::hitmon){
                    env.log("First selection box detected. YES to starter.");
                    seen_selection_arrow = true;
                    pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                }else{
                    env.log("Selection box detected. NO to nickname.");
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                    context.wait_for_all_requests();
                    return;
                }

                if (TARGET == Target::starters){
                    //Skip past energetic and jingle
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                    pbf_wait(context, 500ms);
                    context.wait_for_all_requests();
                }
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);

            }else{
                env.log("Second selection box detected. NO to nickname.");
                pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                //seen_nickname_arrow = true;

                if (TARGET == Target::starters){
                    //Press B some to try and skip the rival's pickup
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                    pbf_press_button(context, BUTTON_B, 320ms, 640ms);
                }
                context.wait_for_all_requests();
                return;
            }
            continue;
        //case 2:
        //    if (seen_selection_arrow){
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

//Different as lapras has multiple no-red-arrow dialog boxes
void GiftReset::obtain_lapras(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Obtaining Lapras.");
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    //At least 9 lines of dialog/No to nickname for Lapras
    //At least 4 lines of dialog for fossils
    //This takes care of the entire conversion+nickname+exit dialog
    int limit = 10;
    if (TARGET == Target::fossils){
        limit = 5;
    }
    for (int i = 0; i < limit; i++){
        pbf_press_button(context, BUTTON_B, 320ms, 640ms);
        pbf_wait(context, 100ms);
        context.wait_for_all_requests();
    }
    context.wait_for_all_requests();
}

//After declining to nickname, clear rival pickup and open your starter's summary
bool GiftReset::try_open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //From no to nickname to overworld
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

    if (TARGET == Target::starters){
        //  We navigate to pokedex since we don't have it yet so it becomes Pokemon.
        if (!move_cursor_to_position(env.console, context, SelectionArrowPositionStartMenu::POKEDEX)){
            std::string str = "open_summary(): Unable to move menu cursor to: " + Pokemon::STRING_POKEMON;
            env.log(str, COLOR_RED);
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                str
            );
            return false;
        }
    }else{
        //Pokedex, Pokemon, Bag, Trainer, Save, Option, Exit
        env.log("Navigating to party menu.");
        pbf_wait(context, 200ms);
        context.wait_for_all_requests();

        if (!move_cursor_to_position(env.console, context, SelectionArrowPositionStartMenu::POKEMON)){
            std::string str = "open_summary(): Unable to move menu cursor to: " + Pokemon::STRING_POKEMON;
            env.log(str, COLOR_RED);
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                str
            );
            return false;
        }
    }

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
    if (TARGET != Target::starters){
        PartySlotWatcher last_slot(COLOR_RED, PartySlot::SIX);
        int ps = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 15; i++) { //Enough to cycle through 6pty+cxl twice
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
        } else{
            env.log("open_summary(): Unable to move selection to slot six.", COLOR_RED);
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "open_summary(): Unable to move selection to slot six."
            );
            return false;
        }
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
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_summary(): Failed to open party summary after 5 attempts.",
        env.console
    );
}


void GiftReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //StartProgramChecks::check_performance_class_wired_or_wireless(context);

    GiftReset_Descriptor::Stats& stats = env.current_stats<GiftReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast. Default borders.
    * Setup: 5 pokemon in your party. None for starter. Stand in front of the pokemon. Save the game.
    * For non-starters: move menu cursor back to the top (POKEDEX)!
    * for starters, pokemon menu will be on top as it is added after picking
    * For magikarp: you need money to buy it
    * fossils: need to corner the scientist
    */

    switch (TARGET){
    case Target::starters:
        env.log("Targeting starters.");
        break;
    case Target::hitmon:
        env.log("Targeting Magikarp, Hitmonlee, and Hitmonchan.");
        break;
    case Target::eevee:
        env.log("Targeting Eevee.");
        break;
    case Target::lapras:
        env.log("Targeting Lapras.");
        break;
    case Target::fossils:
        env.log("Targeting fossils.");
        break;
    default:
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "GiftReset: Invalid target selection.",
            env.console
        );
        break;
    }

    bool shiny_starter = false;

    while (!shiny_starter){
        if (TARGET != Target::lapras && TARGET != Target::fossils){
            obtain_pokemon(env, context);
        }else{
            obtain_lapras(env, context);
        }
        stats.errors += open_summary(env, context);

        VideoSnapshot screen = env.console.video().snapshot();

        ShinySymbolDetector shiny_checker(COLOR_YELLOW);
        shiny_starter = shiny_checker.read(env.console.logger(), screen);

        if (shiny_starter){
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

