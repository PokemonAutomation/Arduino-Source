/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV_RideCloner-1.0.1.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


RideCloner101_Descriptor::RideCloner101_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:RideCloner1.0.1",
        STRING_POKEMON + " SV", "Ride Cloner (1.0.1)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/RideCloner-101.md",
        "Clone your ride legendary (and its item) using the add-to-party glitch.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct RideCloner101_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
//        , m_raids(m_stats["Raids"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_shinies(m_stats["Shinies"])
        , m_cloned(m_stats["Cloned"])
        , m_failed(m_stats["Failed"])
    {
        m_display_order.emplace_back("Date Skips");
//        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Cloned");
        m_display_order.emplace_back("Failed", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_skips;
//    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_cloned;
    std::atomic<uint64_t>& m_failed;
};
std::unique_ptr<StatsTracker> RideCloner101_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



RideCloner101::RideCloner101()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockMode::UNLOCK_WHILE_RUNNING
    )
    , MODE(
        "<b>Mode:</b>",
        {
            {Mode::CLONE_ONLY,  "clone-only",   "Clone only. Don't stop on a shiny raid."},
            {Mode::SHINY_HUNT,  "shiny-hunt",   "Shiny Hunt: Save before each raid and catch. Stop if shiny."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::SHINY_HUNT
    )
    , RIDES_TO_CLONE(
        "<b>Rides to Clone:</b><br>Stop program after cloning this many times. Make sure you have enough box space for twice this amount.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 1, 100
    )
    , MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars to save time since you're likely to lose.",
        LockMode::UNLOCK_WHILE_RUNNING,
        4, 1, 7
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "poke-ball"
    )
    , FIX_TIME_ON_CATCH(
        "<b>Fix Clock on Catch:</b><br>Fix the time when catching so the caught date will be correct.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , A_TO_B_DELAY0(
        "<b>A-to-B Delay:</b><br>The delay between the critical A-to-B press that activates the glitch.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "64ms"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_NONSHINY(
        "Non-Shiny Encounter",
        true, false,
        {"Notifs"},
        std::chrono::seconds(3600)
    )
    , NOTIFICATION_SHINY(
        "Shiny Encounter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_NONSHINY,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(RIDES_TO_CLONE);
    PA_ADD_OPTION(MAX_STARS);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FIX_TIME_ON_CATCH);
    PA_ADD_OPTION(A_TO_B_DELAY0);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(NOTIFICATIONS);
}



//  Start from the overworld with 5 (non-ride legendary) Pokemon in your
//  party. Move your ride legendary into the 6th slot in your party.
void RideCloner101::setup(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Running setup...");

    bool in_party = false;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(info, stream, "RideCloneSetupFailed", "Failed to setup after 5 minutes.");
        }

        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        AdvanceDialogWatcher advance(COLOR_PURPLE);
        PromptDialogWatcher prompt(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {
                overworld,
                main_menu,
                advance,
                prompt,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        VideoSnapshot snapshot = stream.video().snapshot();

        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            if (in_party){
                return;
            }
//            pbf_press_button(context, BUTTON_PLUS, 20, 230);
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            stream.log("Detected main menu.");
            if (advance.detect(snapshot)){
                //  If we detect both the dialog and the main menu, it means we
                //  are selecting who in the party to replace with the ride legendary.
                main_menu.move_cursor(info, stream, context, MenuSide::LEFT, 5, false);
                pbf_press_button(context, BUTTON_A, 20, 105);
                in_party = true;
            }else{
                //  Otherwise we try to move the ride legendary to the party.
                if (main_menu.move_cursor(info, stream, context, MenuSide::LEFT, 6, false)){
                    //  Success, continue.
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }else{
                    //  Failed. It's already in our party.
                    pbf_press_button(context, BUTTON_B, 20, 105);
                    in_party = true;
                }
            }
            continue;
        case 2:
            stream.log("Detected dialog.");
            if (main_menu.detect(snapshot)){
                //  If we detect both the dialog and the main menu, it means we
                //  are selecting who in the party to replace with the ride legendary.
                main_menu.move_cursor(info, stream, context, MenuSide::LEFT, 5, false);
                pbf_press_button(context, BUTTON_A, 20, 105);
                in_party = true;
            }else{
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
            continue;
        case 3:
            stream.log("Detected prompt.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        default:
            dump_image_and_throw_recoverable_exception(info, stream, "RideCloneSetupFailed",
                "setup(): No recognized state after 60 seconds.");
        }
    }
}
bool RideCloner101::run_post_win(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    ProControllerContext& context
){
    console.log("Running post-win...");

    RideCloner101_Descriptor::Stats& stats = env.current_stats<RideCloner101_Descriptor::Stats>();

    if (FIX_TIME_ON_CATCH){
        go_home(console, context);
        home_to_date_time(console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(console, context);
    }

    TeraResult result = TeraResult::NO_DETECTION;
    VideoSnapshot screenshot;
    bool add_to_party_menu = false;
    bool success = false;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(),
                console,
                "RideCloneReturnToOverworldFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA,
            {0.8, 0.93, 0.2, 0.07},
            WhiteButtonWatcher::FinderType::PRESENT,
            std::chrono::seconds(1)
        );
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        PromptDialogWatcher add_to_party(COLOR_PURPLE, {0.500, 0.395, 0.400, 0.100});
        PromptDialogWatcher nickname(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});
        PokemonSummaryWatcher summary(COLOR_MAGENTA);
        MainMenuWatcher main_menu(COLOR_BLUE);
        OverworldWatcher overworld(console.logger(), COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {
                catch_menu,
                next_button,
                advance,
                add_to_party,
                nickname,
                summary,
                main_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            console.log("Detected catch prompt.");
            screenshot = console.video().snapshot();

            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(console, LANGUAGE);
            int quantity = move_to_ball(reader, console, context, BALL_SELECT.slug());
            if (quantity == 0){
                throw_and_log<FatalProgramException>(
                    console.logger(), ErrorReport::NO_ERROR_REPORT,
                    "Unable to find appropriate ball. Did you run out?",
                    console
                );
            }
            if (quantity < 0){
                console.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Detected add-to-party prompt.");
            add_to_party_menu = true;
            if (result == TeraResult::NO_DETECTION){
                pbf_press_dpad(context, DPAD_DOWN, 20, 60);
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 4:
            console.log("Detected prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            if (add_to_party_menu){
                success = true;
            }
            continue;
        case 1:
            //  Next button detector is unreliable. Check if the summary is
            //  open. If so, fall-through to that.
            if (!summary.detect(console.video().snapshot())){
                console.log("Detected possible (A) Next button.");
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            }
            console.log("Detected false positive (A) Next button.", COLOR_RED);
        case 5:
            console.log("Detected summary.");
            if (result == TeraResult::NO_DETECTION){
                context.wait_for(std::chrono::milliseconds(500));
                result = run_tera_summary(
                    env, console, context,
                    NOTIFICATION_NONSHINY,
                    NOTIFICATION_SHINY,
                    MODE == Mode::SHINY_HUNT, screenshot,
                    &stats.m_shinies
                );
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 6:
            console.log("Detected party swap.");
//            context.wait_for(std::chrono::milliseconds(150));
            try{
                if (main_menu.move_cursor(env.program_info(), console, context, MenuSide::LEFT, 5, false)){
                    ssf_press_button(context, BUTTON_A, A_TO_B_DELAY0, 160ms);
                    pbf_press_button(context, BUTTON_B, 20, 230);
                }
            }catch (OperationFailedException& e){
                e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
            }
            continue;
        case 7:
            console.log("Detected overworld.");
            break;
        default:
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "FailedPostRaidWin",
                "run_post_win(): No recognized state after 60 seconds."
            );
        }
        break;
    }




    return success;
}













void RideCloner101::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    assert_16_9_720p_min(env.logger(), env.console);

    RideCloner101_Descriptor::Stats& stats = env.current_stats<RideCloner101_Descriptor::Stats>();


    bool first = true;
    size_t move_counter = 0;
    for (uint16_t items = 0; items < RIDES_TO_CLONE;){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        setup(env.program_info(), env.console, context);

        //  Find raid.
        while (true){
            env.update_stats();
            if (!first){
                day_skip_from_overworld(env.console, context);
                pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY0);
                context.wait_for_all_requests();
                stats.m_skips++;
            }
            first = false;

            if (!open_raid(env.console, context)){
                continue;
            }
            context.wait_for(std::chrono::milliseconds(500));

            VideoSnapshot screen = env.console.video().snapshot();
            TeraCardReader reader(COLOR_RED);
            size_t stars = reader.stars(env.logger(),env.program_info(), screen);
            if (stars != 0){
                env.log("Detected " + std::to_string(stars) + " star raid.", COLOR_PURPLE);
            }

            if (stars > MAX_STARS){
                env.log("Skipping raid...", COLOR_ORANGE);
                stats.m_skipped++;
                close_raid(env.program_info(), env.console, context);
                continue;
            }

            //  Reopen the raid if we need to do stuff.
            bool fix_position = move_counter % 5 == 1;
            Mode mode = MODE;
            if (fix_position || mode == Mode::SHINY_HUNT){
                close_raid(env.program_info(), env.console, context);

                //  Save game because we're shiny-hunting.
                if (mode == Mode::SHINY_HUNT){
                    save_game_from_overworld(env.program_info(), env.console, context);
                }

                //  Fix our position.
                if (fix_position){
                    pbf_move_left_joystick(context, 128, 255, 150, 0);
                    pbf_move_left_joystick(context, 128, 0, 200, 0);
                }

                context.wait_for_all_requests();
                if (open_raid(env.console, context)){
                    env.log("Tera raid found!", COLOR_BLUE);
                }else{
                    env.log("No Tera raid found.", COLOR_ORANGE);
                    continue;
                }
            }

            pbf_press_dpad(context, DPAD_DOWN, 10, 10);
            pbf_mash_button(context, BUTTON_A, 250);

            bool win = run_tera_battle(env, env.console, context, BATTLE_AI);

            if (win){
                stats.m_wins++;
            }else{
                stats.m_losses++;
                context.wait_for(std::chrono::seconds(3));
                continue;
            }

            break;
        }

        if (run_post_win(env, env.console, context)){
            items++;
            stats.m_cloned++;
            pbf_press_button(context, BUTTON_PLUS, 20, 230);
        }else{
            stats.m_failed++;
        }

        move_counter++;
    }


    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
