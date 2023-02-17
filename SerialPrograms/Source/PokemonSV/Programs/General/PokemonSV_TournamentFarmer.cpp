/*  Tournament Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Inference/PokemonSV_TournamentPrizeNameReader.h"
#include "PokemonSV/Resources/PokemonSV_TournamentPrizeNames.h"
#include "PokemonSV_TournamentFarmer.h"
#include "CommonFramework/Tools/ErrorDumper.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;

TournamentFarmer_Descriptor::TournamentFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TournamentFarmer",
        STRING_POKEMON + " SV", "Tournament Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TournamentFarmer.md",
        "Farm the Academy Ace Tournament for money and prizes.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct TournamentFarmer_Descriptor::Stats : public StatsTracker {
    Stats()
        : tournaments(m_stats["Tournaments won"])
        , battles(m_stats["Battles fought"])
        , losses(m_stats["Losses"])
        , money(m_stats["Money made"])
        , matches(m_stats["Items matched"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Tournaments won");
        m_display_order.emplace_back("Battles fought");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Money made");
        m_display_order.emplace_back("Items matched");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& tournaments;
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& losses;
    std::atomic<uint64_t>& money;
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> TournamentFarmer_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}

TournamentFarmer::TournamentFarmer()
    : NUM_ROUNDS(
          "<b>Number of Tournaments to run:",
          LockWhileRunning::UNLOCKED,
          100, 0
          )
    , TRY_TO_TERASTILLIZE(
          "<b>Use Terastillization:</b><br>Tera at the start of battle. Will take longer but may be worth the attack boost.",
          LockWhileRunning::UNLOCKED,
          false
          )
    , SAVE_NUM_ROUNDS(
          "<b>Save every this many tournaments:</b><br>Zero disables saving. Will save win or lose.",
          LockWhileRunning::UNLOCKED,
          0, 0
          )
    , MONEY_LIMIT(
        "<b>Stop after earning this amount of money:</b><br>Zero disables this check. Does not count losses. Max is 999,999,999.",
        LockWhileRunning::UNLOCKED,
        999999999, 0, 999999999
    )
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
          "<b>Game Language:</b><br>The language is needed to read the prizes.",
          TournamentPrizeNameReader::instance().languages(),
          LockWhileRunning::LOCKED,
          true
          )
    , TARGET_ITEMS("<b>Items:</b>")
    , NOTIFICATION_PRIZE_MATCH("Matching Prize", true, false, ImageAttachmentMode::JPG, { "Notifs" })
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_PRIZE_MATCH,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(TRY_TO_TERASTILLIZE);
    PA_ADD_OPTION(SAVE_NUM_ROUNDS);
    PA_ADD_OPTION(MONEY_LIMIT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ITEMS);
    PA_ADD_OPTION(NOTIFICATIONS);
}

//Check and process the amount of money earned at the end of a battle
void TournamentFarmer::check_money(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    int top_money = -1;
    //int bottom_money = -1;

    //There must be a value for top money
    //Bottom money only appear after 1st battle and should clear out
    for (uint16_t c = 0; c < 5 && top_money == -1; c++) {
        VideoSnapshot screen = env.console.video().snapshot();
        ImageFloatBox top_notif(0.745, 0.152, 0.206, 0.083);
        //ImageFloatBox bottom_notif(0.745, 0.261, 0.220, 0.083);

        ImageViewRGB32 image_top = extract_box_reference(screen, top_notif);
        //ImageViewRGB32 image_bottom = extract_box_reference(screen, bottom_notif);

        top_money = OCR::read_number(env.console, image_top);
        //bottom_money = OCR::read_number(env.console, image_bottom);

        dump_image(
            env.console, env.program_info(),
            "battledone",
            screen
        );

        //Filter out low and high numbers as misreads tend to be 44
        //Nemona is lowest at 8640
        //Penny and Geeta are 16800
        if (top_money < 8000 || top_money > 100000 ) {
            top_money = -1;
        }
        //if (bottom_money < 8000 || bottom_money > 100000) {
        //    bottom_money = -1;
        //}
    }

    if (top_money != -1) {
        stats.money += top_money;
        env.update_stats();
    }
    else {
        env.log("Unable to read money.");
    }

}


//Handle a single battle by mashing A until AdvanceDialog (end of battle) is detected
void TournamentFarmer::run_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();
    //Assuming the player has a charged orb
    if (TRY_TO_TERASTILLIZE) {
        //Open move menu
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        pbf_press_button(context, BUTTON_R, 20, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
    }

    //Mash A until battle finished
    AdvanceDialogWatcher end_of_battle(COLOR_YELLOW);
    WallClock start = current_time();
    int ret_black = run_until(
        env.console, context,
        [&](BotBaseContext& context) {
            for(size_t c = 0; c < 30; c++) { //Sylveon build has 16 PP at max, and Chi-Yu build has 24.
                if (current_time() - start > std::chrono::minutes(5)) {
                    env.log("Timed out during battle after 5 minutes.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Timed out during battle after 5 minutes.",
                        true
                    );
                }

                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(60), //Tera takes ~25 to 30 seconds for player/opponent
                    { battle_menu } //End of battle from tera'd ace takes longer, 45 seconds was not enough
                );
                if (ret == 0) {
                    pbf_mash_button(context, BUTTON_A, 300);
                    context.wait_for_all_requests();
                } else {
                    env.log("Timed out during battle. Stuck, crashed, or took more than 60 seconds for a turn.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Timed out during battle. Stuck, crashed, or took more than 60 seconds for a turn.",
                        true
                    );
                }
            }
        },
        { end_of_battle }
        );
    if (ret_black == 0) {
        env.log("Battle finished.");
        stats.battles++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //Close dialog and then check money
        pbf_press_button(context, BUTTON_B, 10, 50);
        pbf_wait(context, 150);
        context.wait_for_all_requests();

        check_money(env, context);

        //Clear any remaining dialog
        pbf_mash_button(context, BUTTON_B, 300);
        context.wait_for_all_requests();
    } else {
        env.log("Timed out during battle. Stuck, crashed, or took over 30 turns.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Timed out during battle. Stuck, crashed, or took over 30 turns.",
            true
        );
    }
    context.wait_for_all_requests();
}


//Check prize and notify if it matches filters after a tournament win
void TournamentFarmer::check_prize(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox dialog_box(0.259, 0.734, 0.484, 0.158);
    ImageViewRGB32 dialog_image = extract_box_reference(screen, dialog_box);

    //bool replace_color_within_range = false;
    //ImageRGB32 dialog_filtered = filter_rgb32_range(
    //    extract_box_reference(screen, dialog_box),
    //    combine_rgb(50, 135, 162), combine_rgb(167, 244, 255), Color(0), replace_color_within_range
    //);
    //dialog_filtered.save("./dialog_image.png");

    const double LOG10P_THRESHOLD = -1.5;
    OCR::StringMatchResult result = PokemonSV::TournamentPrizeNameReader::instance().read_substring(
        env.console, LANGUAGE, dialog_image,
        OCR::BLUE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(LOG10P_THRESHOLD);
    if (result.results.empty()) {
        env.log("No matching prize name found in dialog box.");
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
    for (const auto& r : result.results) {
        env.console.log("Found prize: " + r.second.token);
        if (TARGET_ITEMS.find_item(r.second.token)) {
            env.log("Prize matched");

            stats.matches++;
            env.update_stats();

            send_program_notification(
                env, NOTIFICATION_PRIZE_MATCH,
                COLOR_GREEN, "Prize matched",
                {
                    { "Item:", get_tournament_prize_name(r.second.token).display_name() },
                }
            , "", screen);
            break;
        }
    }
}


//Tournament won and over
void TournamentFarmer::handle_end_of_tournament(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    //Space out the black screen detection after the "champion" battle
    pbf_wait(context, 700);
    context.wait_for_all_requests();

    //One more black screen when done to load the academy
    BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
    int ret_black_won = run_until(
        env.console, context,
        [](BotBaseContext& context) {
            pbf_mash_button(context, BUTTON_A, 10000);
        },
        { black_screen }
        );
    if (ret_black_won == 0) {
        env.log("Tournament complete, waiting for academy.");
    }
    context.wait_for_all_requests();

    //Wait for congrats dialog - wait an extra bit since the dialog appears while still loading in
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    int ret_dialog = wait_until(env.console, context, Milliseconds(1000), { advance_detector });
    if (ret_dialog == 0) {
        env.log("Dialog detected.");
    }
    pbf_wait(context, 300);
    context.wait_for_all_requests();

    //Next dialog is prize dialog - wait until all dialog has appeared then check prize
    pbf_press_button(context, BUTTON_A, 10, 100);
    pbf_wait(context, 100);
    context.wait_for_all_requests();

    check_prize(env, context);

    //Clear remaining dialog
    OverworldWatcher overworld(COLOR_CYAN);
    int ret_over = run_until(
        env.console, context,
        [](BotBaseContext& context) {
            pbf_mash_button(context, BUTTON_B, 700);
        },
        { overworld }
        );
    if (ret_over != 0) {
        env.console.log("Failed to detect overworld.", COLOR_RED);
    }
    context.wait_for_all_requests();

    stats.tournaments++;
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
}


//Fly to academy from west pokemon center after losing.
void TournamentFarmer::return_to_academy_after_loss(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    env.log("Tournament lost! Navigating back to academy.");
    open_map_from_overworld(env.program_info(), env.console, context);
    pbf_press_button(context, BUTTON_ZR, 50, 40);
    pbf_move_left_joystick(context, 187, 0, 50, 0);
    fly_to_overworld_from_map(env.program_info(), env.console, context);

    pbf_wait(context, 100);
    context.wait_for_all_requests();

    env.log("At academy fly point. Heading back to doors.");
    pbf_move_left_joystick(context, 0, 128, 8, 0);
    pbf_press_button(context, BUTTON_L, 50, 40);
    pbf_press_button(context, BUTTON_PLUS, 50, 40);
    pbf_press_button(context, BUTTON_B, 50, 40); //Trying to jump/glide over npc spawns
    pbf_press_button(context, BUTTON_B, 50, 40);
    pbf_move_left_joystick(context, 128, 0, 500, 0);
    pbf_press_button(context, BUTTON_B, 50, 40);
    pbf_press_button(context, BUTTON_B, 50, 40);

    BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
    int ret_black_lost = run_until(
        env.console, context,
        [](BotBaseContext& context) {
            pbf_move_left_joystick(context, 128, 0, 5000, 0);
        },
        { black_screen }
        );
    context.wait_for_all_requests();
    if (ret_black_lost == 0) {
        env.log("Black screen detected.");
    }

    //Wait for academy to load.
    OverworldWatcher overworld(COLOR_CYAN);
    int ret_academy = wait_until(env.console, context, Milliseconds(4000), { overworld });
    if (ret_academy == 0) {
        env.log("Entered academy. Walking to tournament entry.");
    }
    context.wait_for_all_requests();

    //Move to tournament entry
    pbf_move_left_joystick(context, 128, 0, 500, 0);
    pbf_move_left_joystick(context, 0, 128, 100, 0);
    pbf_move_left_joystick(context, 255, 0, 100, 0);
    context.wait_for_all_requests();
}


void TournamentFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    assert_16_9_720p_min(env.logger(), env.console);
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    /*
    Preconditions:
    Last Pokemon Center visited is Mesagzoa West
    Sylveon only farming build - ideally with fairy tera
    stand in front of tournament entry man
    Ride legendary is not the solo pokemon (in case of loss)
    Do not have other notifications on screen for money reading (ex. new outbreak)

    Possible improvements to make:
    find prize sprites - some code is there, just disabled
    find translations for tera shards
    other languages: make sure "bottle cap" isn't misread as "bottle of PP Up"
    */

    for(uint32_t c = 0; c < NUM_ROUNDS; c++) {
        //Initiate dialog then mash until first battle starts
        AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
        pbf_press_button(context, BUTTON_A, 10, 50);
        int ret = wait_until(env.console, context, Milliseconds(7000), { advance_detector });
        if (ret < 0) {
            env.log("Dialog detected.");
        }
        pbf_mash_button(context, BUTTON_A, 300);
        context.wait_for_all_requests();

        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        int ret_battle = run_until(
            env.console, context,
            [](BotBaseContext& context) {
                pbf_mash_button(context, BUTTON_B, 10000); //it takes a while to load and start
            },
            { battle_menu }
            );
        if (ret_battle != 0) {
            env.console.log("Failed to detect battle start!", COLOR_RED);
        }
        context.wait_for_all_requests();

        bool battle_lost = false;
        for (uint16_t battles = 0; battles < 4; battles++) {
            NormalBattleMenuWatcher battle_menu2(COLOR_YELLOW); //Next battle started
            OverworldWatcher overworld(COLOR_CYAN); //Previous battle was lost
            
            int ret_battle2 = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_mash_button(context, BUTTON_B, 4500);
                },
                { battle_menu2, overworld }
                );
            context.wait_for_all_requests();

            switch (ret_battle2) {
            case 0:
                env.log("Detected battle menu.");
                run_battle(env, context);
                break;
            case 1:
                env.log("Detected overworld.");
                battle_lost = true;
                stats.losses++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                break;
            default:
                env.log("Failed to detect battle menu or dialog prompt!");
                stats.errors++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                break;
            }

            if (battle_lost) {
                return_to_academy_after_loss(env, context);
                break;
            }
        }

        //Tournament won
        if (!battle_lost) {
            handle_end_of_tournament(env, context);
        }

        //Save the game if option is set
        uint16_t num_rounds_temp = SAVE_NUM_ROUNDS;
        if (num_rounds_temp != 0 && ((c + 1) % num_rounds_temp) == 0) {
            env.log("Saving game.");
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        //Break loop and finish program if money limit is hit
        uint32_t earnings_temp = MONEY_LIMIT;
        if (earnings_temp != 0 && stats.money >= earnings_temp) {
            env.log("Money limit hit. Ending program.");
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}

