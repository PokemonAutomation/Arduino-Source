/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.h"
//#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV_TeraSelfFarmer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


TeraSelfFarmer_Descriptor::TeraSelfFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TeraSelfFarmer",
        STRING_POKEMON + " SV", "Tera Self Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TeraSelfFarmer.md",
        "Farm items and " + STRING_POKEMON + " from Tera raids. Can also hunt for shiny and high reward raids.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct TeraSelfFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
        , m_raids(m_stats["Raids"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_caught(m_stats["Caught"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Date Skips");
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Caught", true);
        m_display_order.emplace_back("Shinies", true);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_caught;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> TeraSelfFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



TeraFarmerOpponentFilter::TeraFarmerOpponentFilter()
    : GroupOption("Opponent Filter", LockWhileRunning::UNLOCKED)
    , MIN_STARS(
        "<b>Min Stars:</b><br>Skip raids with less than this many stars.",
        LockWhileRunning::UNLOCKED,
        1, 1, 7
    )
    , MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars to save time since you're likely to lose.",
        LockWhileRunning::UNLOCKED,
        4, 1, 7
    )
{
    PA_ADD_OPTION(MIN_STARS);
    PA_ADD_OPTION(MAX_STARS);
}
bool TeraFarmerOpponentFilter::should_battle(size_t stars) const{
    return MIN_STARS <= stars && stars <= MAX_STARS;
}

TeraFarmerCatchOnWin::TeraFarmerCatchOnWin(TeraSelfFarmer& program)
    : GroupOption("Catch on Win - Required for shiny checking.", LockWhileRunning::UNLOCKED, true)
    , m_program(program)
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
    , FIX_TIME_ON_CATCH(
        "<b>Fix Clock:</b><br>Fix the time when catching so the caught date will be correct.",
        LockWhileRunning::UNLOCKED, false
    )
{
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FIX_TIME_ON_CATCH);
}
void TeraFarmerCatchOnWin::on_set_enabled(bool enabled){
    m_program.STOP_CONDITIONS.STOP_ON_SHINY.set_visibility(
        enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED
    );
}

TeraFarmerStopConditions::TeraFarmerStopConditions()
    : GroupOption("Stop Conditions", LockWhileRunning::UNLOCKED)
    , MAX_CATCHES(
        "<b>Max Catches:</b><br>Stop program after catching this many " + STRING_POKEMON + ".",
        LockWhileRunning::UNLOCKED,
        50, 1, 999
    )
    , STOP_ON_SHINY(
        "<b>Stop on Shiny:</b> (requires catching the " + STRING_POKEMON + ")<br>"
        "Stop the program if a shiny is found. Resetting the game will return you to the front of this (shiny) raid so it can be hosted again.",
        LockWhileRunning::UNLOCKED,
        true
    )
    , STOP_ON_RARE_ITEMS(
        "<b>Stop on Rare Items:</b><br>"
        "Stop the program if the rewards contain at least this many rare (sparkly) items. Set to zero to disable this feature and never stop for item rewards.<br>"
        "Note that the program can only see the first 8 item rewards. It will not scroll down.",
        LockWhileRunning::UNLOCKED,
        0, 0, 8
    )
{
    PA_ADD_OPTION(MAX_CATCHES);
    PA_ADD_OPTION(STOP_ON_SHINY);
    PA_ADD_OPTION(STOP_ON_RARE_ITEMS);
}


TeraSelfFarmer::TeraSelfFarmer()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockWhileRunning::UNLOCKED
    )
    , CATCH_ON_WIN(*this)
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
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(CATCH_ON_WIN);
    PA_ADD_OPTION(STOP_CONDITIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool TeraSelfFarmer::run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Running raid...");

    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    bool win = run_tera_battle(
        env, env.console, context,
        NOTIFICATION_ERROR_RECOVERABLE,
        BATTLE_AI
    );

    if (win){
        stats.m_wins++;
    }else{
        stats.m_losses++;
        context.wait_for(std::chrono::seconds(3));
        return false;
    }

//    if (MODE == Mode::FARM_ITEMS_ONLY){
    if (!CATCH_ON_WIN.enabled()){
        exit_tera_win_without_catching(env.program_info(), env.console, context, 0);
        return true;
    }

    VideoSnapshot battle_snapshot = env.console.video().snapshot();


    if (CATCH_ON_WIN.FIX_TIME_ON_CATCH){
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        resume_game_from_home(env.console, context);
    }

    m_number_caught++;
    stats.m_caught++;

    exit_tera_win_by_catching(
        env, env.console, context,
        LANGUAGE,
        CATCH_ON_WIN.BALL_SELECT.slug(),
        NOTIFICATION_NONSHINY,
        NOTIFICATION_SHINY,
        STOP_CONDITIONS.STOP_ON_SHINY,
        STOP_CONDITIONS.STOP_ON_RARE_ITEMS,
        &stats.m_shinies
    );
    return true;
}


void TeraSelfFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    if (FILTER.MIN_STARS > FILTER.MAX_STARS){
        throw UserSetupError(env.console, "Error in the settings, \"Min Stars\" is bigger than \"Max Stars\".");
    }

    m_number_caught = 0;

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 10);

    bool first = true;

    while (true){
        if (m_number_caught >= STOP_CONDITIONS.MAX_CATCHES){
            break;
        }

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);


        //  Clear per-iteration flags.
//        m_battle_finished = false;
//        m_caught = false;
//        m_summary_read = false;


        if (!first){
            day_skip_from_overworld(env.console, context);
            pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY);
            context.wait_for_all_requests();
            stats.m_skips++;
        }
        first = false;

        if (open_raid(env.console, context)){
//            env.log("Tera raid found!", COLOR_BLUE);
            stats.m_raids++;
        }else{
//            env.log("No Tera raid found.", COLOR_ORANGE);
            continue;
        }
        context.wait_for(std::chrono::milliseconds(500));

        VideoSnapshot screen = env.console.video().snapshot();
        TeraCardReader reader(COLOR_RED);
        size_t stars = reader.stars(screen);
        if (stars == 0){
            dump_image(env.logger(), env.program_info(), "ReadStarsFailed", *screen.frame);
        }

        VideoOverlaySet overlay_set(env.console);

        TeraSilhouetteReader silhouette_reader;
        silhouette_reader.make_overlays(overlay_set);
        ImageMatch::ImageMatchResult silhouette = silhouette_reader.read(screen);
        silhouette.log(env.logger(), 100);
        std::string best_silhouette = silhouette.results.empty() ? "UnknownSilhouette" : silhouette.results.begin()->second;
        if (silhouette.results.empty()){
            dump_image(env.logger(), env.program_info(), "ReadSilhouetteFailed", *screen.frame);
        }
        else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(env.logger(), "PokemonSV/TeraSelfFarmer/" + best_silhouette, "", screen);
        }

        TeraTypeReader type_reader;
        type_reader.make_overlays(overlay_set);
        ImageMatch::ImageMatchResult type = type_reader.read(screen);
        type.log(env.logger(), 100);
        std::string best_type = type.results.empty() ? "UnknownType" : type.results.begin()->second;
        if (type.results.empty()){
            dump_image(env.logger(), env.program_info(), "ReadTypeFailed", *screen.frame);
        }
        else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(env.logger(), "PokemonSV/TeraSelfFarmer/" + best_type, "", screen);
        }

        {
            std::string log = "Detected a " + std::to_string(stars) + "* " + best_type + " " + best_silhouette;
            env.console.overlay().add_log(log, COLOR_GREEN);
            env.log(log);
        }

        if (!FILTER.should_battle(stars)) {
            env.log("Skipping raid...", COLOR_ORANGE);
            stats.m_skipped++;
            close_raid(env.program_info(), env.console, context);
            continue;
        }

//        if (MODE == Mode::SHINY_HUNT){
        if (true){
            close_raid(env.program_info(), env.console, context);
            save_game_from_overworld(env.program_info(), env.console, context);
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
        bool raid_won = run_raid(env, context);
        {
            std::stringstream ss;
            ss << "You ";
            if (raid_won){
                ss << "won";
            }
            else{
                ss << "lost";
            }
            ss << " a " << stars << "* " << best_type << " " << best_silhouette << " raid";
            env.log(ss.str());
            env.console.overlay().add_log(ss.str(), COLOR_GREEN);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}











}
}
}
