/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
//#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
//#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV_TeraSelfFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


TeraSelfFarmer_Descriptor::TeraSelfFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TeraSelfFarmer",
        STRING_POKEMON + " BDSP", "Tera Self Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TeraSelfFarmer.md",
        "Farm items from Tera raids.",
        FeedbackType::REQUIRED, false,
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
    {
        m_display_order.emplace_back("Date Skips");
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> TeraSelfFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



TeraSelfFarmer::TeraSelfFarmer()
    : MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars to save time since you're likely to lose.",
        LockWhileRunning::UNLOCKED,
        4, 1, 7
    )
#if 0
    , WIN_ACTION(
        "<b>Win Action:</b>",
        {
            {WinAction::CATCH,      "catch", "Catch"},
            {WinAction::DONT_CATCH, "leave", "Do not catch."},
        },
        LockWhileRunning::UNLOCKED,
        WinAction::CATCH
    )
#endif
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
//        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , RAID_SPAWN_DELAY(
        "<b>Raid Spawn Delay</b>",
        LockWhileRunning::UNLOCKED,
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(MAX_STARS);
//    PA_ADD_OPTION(WIN_ACTION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void TeraSelfFarmer::run_raid(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    while (true){
        context.wait_for_all_requests();

        BattleMenuFinder battle_menu;
        MoveSelectFinder move_select_menu;
        GradientArrowFinder target_select_menu(env.console.overlay(), {0.45, 0.07, 0.10, 0.10});
        TeraCatchFinder catch_menu;
        BlackScreenOverWatcher black_screen;
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            {
                battle_menu,
                move_select_menu,
                target_select_menu,
                catch_menu,
                black_screen,
            }
        );
        switch (ret){
        case 0:
            env.log("Detected battle menu!");
            break;
        case 1:
            env.log("Unexpected move select.", COLOR_RED);
            break;
        case 2:
            env.log("Unexpected target select menu.", COLOR_RED);
            break;
        case 3:
            env.log("Detected a win!", COLOR_BLUE);
            stats.m_wins++;
            goto Done;
        case 4:
            env.log("Detected a loss!", COLOR_ORANGE);
            stats.m_losses++;
            context.wait_for(std::chrono::seconds(3));
            return;
        default:
            dump_image_and_throw_recoverable_exception(
                env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                "BattleMenuNotFound",
                "Unable to detect Tera raid battle menu after 60 seconds."
            );
        }

        pbf_mash_button(context, BUTTON_A, 375);
    }

Done:

//    if (WIN_ACTION == WinAction::DONT_CATCH){
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
//    }

    {
        WhiteButtonFinder next_button(WhiteButton::ButtonA, env.console.overlay(), {0.9, 0.9, 0.1, 0.1});
        pbf_mash_button(context, BUTTON_A, 250);
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(60),
            {next_button}
        );
        if (ret < 0){
            dump_image_and_throw_recoverable_exception(
                env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                "BattleMenuNotFound",
                "Unable to detect prize screen after 60 seconds."
            );
        }
        env.log("Detected (A) Next button.");
    }
    pbf_mash_button(context, BUTTON_A, 125);

    {
        env.log("Waiting for black screen.");
        BlackScreenOverWatcher black_screen;
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(60),
            {black_screen}
        );
        if (ret < 0){
            dump_image_and_throw_recoverable_exception(
                env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                "BattleMenuNotFound",
                "Unable to detect end of battle after 60 seconds."
            );
        }

        env.log("Waiting for white screen.");
        WhiteScreenOverWatcher white_screen;
        ret = wait_until(
            env.console, context,
            std::chrono::seconds(60),
            {white_screen}
        );
        if (ret < 0){
            dump_image_and_throw_recoverable_exception(
                env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                "BattleMenuNotFound",
                "Unable to detect end of battle after 60 seconds."
            );
        }
    }

#if 0
    {
        GradientArrowFinder gradient(env.console.overlay(), {0.40, 0.40, 0.30, 0.10});
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(3),
            {gradient}
        );
        if (ret == 0){
            env.log("Detected post catch.");
        }
    }

    env.log("Clearing final dialogs...");
    DialogDetector dialog;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = env.console.video().snapshot();
        if (dialog.detect(screen)){
            pbf_press_button(context, BUTTON_B, 20, 180);
        }else{
            break;
        }
    }
#endif

    context.wait_for_all_requests();
    context.wait_for(std::chrono::seconds(3));
}

void TeraSelfFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    uint8_t year = MAX_YEAR;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_wait(context, RAID_SPAWN_DELAY);
        context.wait_for_all_requests();

        stats.m_skips++;

        TeraCardFinder card_detector;
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 355);
            },
            {card_detector}
        );
        if (ret < 0){
            env.log("No Tera raid found.", COLOR_ORANGE);
            continue;
        }

        env.log("Tera raid found!", COLOR_BLUE);
        stats.m_raids++;

        VideoSnapshot screen = env.console.video().snapshot();
        size_t stars = card_detector->stars(screen);
        if (stars == 0){
            dump_image(env.logger(), env.program_info(), "ReadStarsFailed", *screen.frame);
        }else{
            env.log("Detected " + std::to_string(stars) + " star raid.", COLOR_PURPLE);
        }

        if (stars <= MAX_STARS){
            pbf_press_dpad(context, DPAD_DOWN, 10, 10);
            pbf_mash_button(context, BUTTON_A, 250);
            run_raid(env, context);
        }else{
            env.log("Skipping raid...", COLOR_ORANGE);
            stats.m_skipped++;
            pbf_press_button(context, BUTTON_B, 20, 230);
        }

    }

}











}
}
}
