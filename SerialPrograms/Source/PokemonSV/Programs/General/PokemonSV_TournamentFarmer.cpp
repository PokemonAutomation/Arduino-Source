/*  Tournament Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
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

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;

TournamentFarmer_Descriptor::TournamentFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TournamentFarmer",
        STRING_POKEMON + " SV", "Tournament Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TournamentFarmer.md",
        "Farm the Ace Academy Tournament for money and prizes.",
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
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Tournaments won");
        m_display_order.emplace_back("Battles fought");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& tournaments;
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& losses;
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
          "<b>Save every this many tournaments:</b><br>(zero disables saving)",
          LockWhileRunning::UNLOCKED,
          0, 0
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
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ITEMS);
    PA_ADD_OPTION(NOTIFICATIONS);
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

    Possible improvements to make:
    money tracking? notifs tend to backlog.
    find prize sprites - some code is there, just disabled
    find translations for mints and tera shards
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

        BlackScreenWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
        bool battle_lost = false;
        for (uint16_t battles = 0; !battle_lost && battles < 4; battles++) {
            //Wait for battle - shorter than tournament start above
            NormalBattleMenuWatcher battle_menu2(COLOR_YELLOW);
            OverworldWatcher overworld(COLOR_CYAN);

            int ret_battle2 = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_mash_button(context, BUTTON_B, 4000);
                },
                { battle_menu2, overworld }
                );
            context.wait_for_all_requests();
            int ret_black;
            switch (ret_battle2) {
            case 0:
                env.log("Detected battle menu.");

                //Assuming the player has a charged orb
                if (TRY_TO_TERASTILLIZE) {
                    //Open move menu
                    pbf_press_button(context, BUTTON_A, 10, 50);
                    pbf_wait(context, 100);
                    context.wait_for_all_requests();

                    pbf_press_button(context, BUTTON_R, 20, 50);
                }

                //Mash A until battle finished
                ret_black = run_until(
                    env.console, context,
                    [](BotBaseContext& context) {
                        pbf_mash_button(context, BUTTON_A, 30000);
                        pbf_mash_button(context, BUTTON_A, 7500); //5 minutes should be more than enough for one battle
                    },
                    { black_screen }
                    );
                if (ret_black == 0) {
                    env.log("Battle finished.");
                    stats.battles++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                }
                else {
                    env.log("Timed out during battle after 5 minutes.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                    throw OperationFailedException(env.console, "Timed out after 5 minutes.", true);
                }
                context.wait_for_all_requests();
                break;
            case 1:
                env.log("Detected overworld.");
                stats.losses++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                battle_lost = true;
                break;
            default:
                env.log("Failed to detect battle menu or dialog prompt!");
                stats.errors++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                break;
            }
        }
        pbf_wait(context, 700); //Space out the black screen detection a bit
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen2(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
        //Tournament won
        if (!battle_lost) {
            //One more black screen when done to load the academy
            int ret_black_won = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_mash_button(context, BUTTON_A, 10000);
                },
                { black_screen2 }
                );
            if (ret_black_won == 0) {
                env.log("Tournament complete, waiting for academy.");
            }
            context.wait_for_all_requests();

            //Wait for congrats dialog - wait an extra bit since the dialog appears while still loading in
            AdvanceDialogWatcher advance_detector2(COLOR_YELLOW);
            int ret_dialog = wait_until(env.console, context, Milliseconds(1000), { advance_detector2 });
            if (ret_dialog == 0) {
                env.log("Dialog detected.");
            }
            pbf_wait(context, 300);
            context.wait_for_all_requests();

            pbf_press_button(context, BUTTON_A, 10, 100);
            pbf_wait(context, 100);
            context.wait_for_all_requests();

            //Detect prize
            //bool replace_color_within_range = false;
            VideoSnapshot screen = env.console.video().snapshot();
            ImageFloatBox dialog_box(0.259, 0.734, 0.484, 0.158);
            ImageViewRGB32 dialog_image = extract_box_reference(screen, dialog_box);

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
                    send_program_notification(
                        env, NOTIFICATION_PRIZE_MATCH,
                        COLOR_GREEN, "Prize matched",
                        {
                            { "Item:", r.second.token },
                        }
                    , "", screen);
                    break;
                }
            }

            //Clear remaining dialog and check if we need to save
            OverworldWatcher overworld2(COLOR_CYAN);
            int ret_over = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_mash_button(context, BUTTON_B, 700);
                },
                { overworld2 }
                );
            if (ret_over != 0) {
                env.console.log("Failed to detect overworld.", COLOR_RED);
            }
            context.wait_for_all_requests();

            //Save the game if option checked, then loop again
            if (SAVE_NUM_ROUNDS != 0 && ((c + 1) % SAVE_NUM_ROUNDS) == 0) {
                env.log("Saving game.");
                save_game_from_overworld(env.program_info(), env.console, context);
            }

            stats.tournaments++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        }
        if (battle_lost) {
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

            int ret_black_lost = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_move_left_joystick(context, 128, 0, 5000, 0);
                },
                { black_screen2 }
                );
            context.wait_for_all_requests();
            if (ret_black_lost == 0) {
                env.log("Black screen detected.");
            }

            //Wait for academy to load.
            OverworldWatcher overworld3(COLOR_CYAN);
            int ret_academy = wait_until(env.console, context, Milliseconds(4000), { overworld3 });
            if (ret_academy == 0) {
                env.log("Entered academy. Walking to tournament entry.");
            }
            context.wait_for_all_requests();
            pbf_move_left_joystick(context, 128, 0, 500, 0);
            pbf_move_left_joystick(context, 0, 128, 100, 0);
            pbf_move_left_joystick(context, 255, 0, 100, 0);
            context.wait_for_all_requests();
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}

