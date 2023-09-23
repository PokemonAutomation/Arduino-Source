/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_StatsResetChecker.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_StatsReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;
 
StatsReset_Descriptor::StatsReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:StatsReset",
        STRING_POKEMON + " SV", "Stats Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/StatsReset.md",
        "Repeatedly catch the Treasures of Ruin or Loyal Three until you get the stats you want.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct StatsReset_Descriptor::Stats : public StatsTracker {
    Stats()
        : resets(m_stats["Resets"])
        , balls(m_stats["Balls Thrown"])
        , catches(m_stats["Catches"])
        , matches(m_stats["Matches"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Balls Thrown");
        m_display_order.emplace_back("Catches");
        m_display_order.emplace_back("Matches");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& balls;
    std::atomic<uint64_t>& catches;
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StatsReset_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}
StatsReset::StatsReset()
    : TARGET(
        "<b>Target:</b><br>The Pokemon you are resetting for.<br>"
        "Treasures of Ruin: Stand in front of the unsealed vaults of one of the Ruinous Quartet.<br>"
        "Loyal Three: Stand in front of Okidogi/Munkidori/Fezandipiti.<br>"
        //"Generic: You are standing in front of a Pokemon that requires an A press to initiate battle.<br>",
        "Gimmighoul: Stand in front of a Gimmighoul chest.<br>",
        {
            {Target::TreasuresOfRuin, "treasures-of-ruin", "Treasures of Ruin"},
            {Target::LoyalThree, "loyal-three", "Loyal Three"},
            {Target::Generic, "generic", "Gimmighoul"},
        },
        LockWhileRunning::LOCKED,
        Target::TreasuresOfRuin
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>This field is required so we can read IVs.",
        IV_READER().languages(),
        LockWhileRunning::LOCKED,
        true
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        & NOTIFICATION_PROGRAM_FINISH,
        & NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(LANGUAGE); //This is required
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FILTERS); //Note: None of these can be shiny, and the quartet will have some perfect IVs.
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StatsReset::enter_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    //Press A to talk to target
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    pbf_press_button(context, BUTTON_A, 10, 50);
    int retD = wait_until(env.console, context, Milliseconds(4000), { advance_detector });
    if (retD < 0) {
        env.log("Dialog not detected.");
    }

    switch (TARGET) {
    case Target::TreasuresOfRuin:
        //~30 seconds to start battle?
        pbf_mash_button(context, BUTTON_A, 3250);
        context.wait_for_all_requests();
        break;
    case Target::LoyalThree:
        //Mash through dialog box
        pbf_mash_button(context, BUTTON_B, 1300);
        context.wait_for_all_requests();
        break;
    case Target::Generic:
        //Mash A to initiate battle
        pbf_mash_button(context, BUTTON_A, 90);
        context.wait_for_all_requests();
        break;
    default:
        throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Unknown Target");
    }

    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
    int ret = wait_until(
        env.console, context,
        std::chrono::seconds(15),
        { battle_menu }
    );
    if (ret != 0) {
        stats.errors++;
        env.update_stats();
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to enter battle. Are you facing the Pokemon or in a menu?",
            true
        );
    }
}

//Returns target_fainted. If overworld is detected then the target fainted.
//Otherwise if AdvanceDialog is detected the Pokemon was caught or the player lost.
bool StatsReset::run_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    AdvanceDialogWatcher advance_dialog(COLOR_MAGENTA);
    OverworldWatcher overworld(COLOR_BLUE);

    uint8_t switch_party_slot = 1;

    bool target_fainted = false;
    bool out_of_balls = false;

    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context) {
            while (true) {
                //Check that battle menu appears - this is in case of swapping pokemon
                NormalBattleMenuWatcher menu_before_throw(COLOR_YELLOW);
                int bMenu = wait_until(
                    env.console, context,
                    std::chrono::seconds(15),
                    { menu_before_throw }
                );
                if (bMenu < 0) {
                    env.console.log("Unable to find menu_before_throw.");
                    stats.errors++;
                    env.update_stats();
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Unable to find menu_before_throw.",
                        true
                    );
                }

                //Navigate to correct ball and repeatedly throw it until caught
                pbf_press_button(context, BUTTON_X, 20, 100);
                context.wait_for_all_requests();

                BattleBallReader reader(env.console, LANGUAGE);
                int quantity = move_to_ball(reader, env.console, context, BALL_SELECT.slug());
                if (quantity == 0) {
                    out_of_balls = true;
                    env.console.log("Unable to find appropriate ball/out of balls.");
                    send_program_status_notification(
                        env, NOTIFICATION_STATUS_UPDATE,
                        "Unable to find appropriate ball/out of balls."
                    );
                    break;
                }
                if (quantity < 0) {
                    stats.errors++;
                    env.update_stats();
                    env.console.log("Unable to read ball quantity.", COLOR_RED);
                }
                //Throw ball
                pbf_mash_button(context, BUTTON_A, 150);
                context.wait_for_all_requests();

                //Check for battle menu
                //If found after a second then assume Chi-Yu used Bounce and is invulnerable
                //Use first attack this turn!
                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(4),
                    { battle_menu }
                );
                if (ret == 0) {
                    env.console.log("Battle menu detected early. Using first attack.");
                    pbf_mash_button(context, BUTTON_A, 250);
                    context.wait_for_all_requests();
                }
                else {
                    //Wild pokemon's turn/wait for catch animation
                    stats.balls++;
                    env.update_stats();
                    pbf_mash_button(context, BUTTON_B, 900);
                    context.wait_for_all_requests();
                }

                OverworldWatcher overworld(COLOR_BLUE);
                SwapMenuWatcher fainted(COLOR_YELLOW);
                int ret2 = wait_until(
                    env.console, context,
                    std::chrono::seconds(60),
                    { battle_menu, fainted }
                );
                switch (ret2) {
                case 0:
                    env.log("Battle menu detected, continuing.");
                    break;
                case 1:
                    env.log("Detected fainted Pokemon. Switching to next living Pokemon...");
                    if (fainted.move_to_slot(env.console, context, switch_party_slot)) {
                        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                        context.wait_for_all_requests();
                        switch_party_slot++;
                    }
                    break;
                default:
                    env.console.log("Invalid state ret2 run_battle.");
                    stats.errors++;
                    env.update_stats();
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Invalid state ret2 run_battle.",
                        true
                    );
                }

            }
        },
        { advance_dialog, overworld }
        );

    switch (ret) {
    case 0:
        env.log("Advance Dialog detected. Either caught target or lost battle.");
        target_fainted = false;
        break;
    case 1:
        env.log("Overworld detected, target Pokemon fainted.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Overworld detected, target Pokemon fainted."
        );
        target_fainted = true;
        break;
    default:
        if (out_of_balls) {
            target_fainted = true; //Resets game.
            env.log("Ran out of selected Pokeball. Resetting.");
            break;
        }
        env.console.log("Invalid state in run_battle().");
        stats.errors++;
        env.update_stats();
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Invalid state in run_battle().",
            true
        );
    }

    return target_fainted;
}

bool StatsReset::check_stats(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();
    bool match = false;

    //Open box
    enter_box_system_from_overworld(env.program_info(), env.console, context);
    context.wait_for(std::chrono::milliseconds(400));

    //Check that the target pokemon was caught
    if (check_empty_slots_in_party(env.program_info(), env.console, context) != 0) {
        env.console.log("One or more empty slots in party. Target was not caught or user setup error.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "One or more empty slots in party. Target was not caught."
        );
    }
    else {
        stats.catches++;
        env.update_stats();

        //Navigate to last party slot
        move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 5, 0);

        //Check the IVs of the newly caught Pokemon - *must be on IV panel*
        EggHatchAction action = EggHatchAction::Keep;
        check_stats_reset_info(env.console, context, LANGUAGE, FILTERS, action);

        switch (action) {
        case EggHatchAction::StopProgram:
            match = true;
            env.console.log("Match found!");
            stats.matches++;
            env.update_stats();
            send_program_status_notification(
                env, NOTIFICATION_PROGRAM_FINISH,
                "Match found!"
            );
            break;
        case EggHatchAction::Release:
            match = false;
            env.console.log("Stats did not match table settings.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Stats did not match table settings."
            );
            break;
        default:
            env.console.log("Invalid state.");
            stats.errors++;
            env.update_stats();
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Invalid state.",
                true
            );
        }
    }

    return match;
}

void StatsReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    //This will only work for Pokemon that you press A to talk to.
    //Regular static spawns will have the same stats, resetting won't work.
    //Won't apply to the former titan pokemon or the box legends + ogrepon either, as their IVs are locked.
    //So this really only applies to the ruinous quartet and loyal three
    //Use first attack if target pokemon is invulnerable (Chi-Yu used Bounce)

    assert_16_9_720p_min(env.logger(), env.console);
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    //Autosave must be off, settings like Tera farmer.
    bool stats_matched = false;
    while (!stats_matched){
        enter_battle(env, context);
        bool target_fainted = run_battle(env, context);

        if (!target_fainted) {
            //Close all the dex entry and caught menus
            //If the player lost, this closes all dialog from Joy
            OverworldWatcher overworld;
            int retOver = run_until(
                env.console, context,
                [](BotBaseContext& context) {
                    pbf_mash_button(context, BUTTON_B, 10000);
                },
                { overworld }
                );
            if (retOver != 0) {
                env.log("Failed to detect overworld.", COLOR_RED);
            }
            else {
                env.log("Detected overworld.");
            }
            context.wait_for_all_requests();

            stats_matched = check_stats(env, context);
        }

        if (target_fainted || !stats_matched) {
            //Reset game
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Resetting game."
            );
            stats.resets++;
            env.update_stats();
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
        }
    }
    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}
    
}
}
}
