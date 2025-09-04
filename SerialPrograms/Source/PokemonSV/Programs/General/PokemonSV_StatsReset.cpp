/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_StatsResetChecker.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
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
        "Repeatedly catch static encounters until you get the stats you want.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StatsReset_Descriptor::Stats : public StatsTracker{
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
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& balls;
    std::atomic<uint64_t>& catches;
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StatsReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}
StatsReset::StatsReset()
    : TARGET(
        "<b>Target:</b><br>The Pokemon you are resetting for.",
        //"Treasures of Ruin: Stand in front of the unsealed vaults of one of the Ruinous Quartet.<br>"
        //"Loyal Three: Stand in front of Okidogi/Munkidori/Fezandipiti.<br>"
        //"Snacksworth Legendary: After unlocking a legendary from Snacksworth, stand in front of it.<br>"
        //"Generic: You are standing in front of a Pokemon that requires an A press to initiate battle.<br>",
        //"Gimmighoul: Stand in front of a Gimmighoul chest.<br>",
        {
            {Target::TreasuresOfRuin, "treasures-of-ruin", "Treasures of Ruin"},
            {Target::LoyalThree, "loyal-three", "Loyal Three"},
            {Target::Snacksworth, "snacksworth", "Snacksworth Legendaries + Meloetta"},
            {Target::Generic, "generic", "Indigo Disk Paradoxes (nature only) + Gimmighoul"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::TreasuresOfRuin
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>This field is required so we can read IVs.",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "poke-ball"
    )
    , QUICKBALL(
        "<b>Throw Quick Ball:</b><br>Use a Quick Ball on the first turn. If there are moves in the Move Table, they will run <i>after</i> the Quick Ball is thrown.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , FILTERS(
        StatsHuntIvJudgeFilterTable_Label_Regular,
        {
            .action = false,
            .shiny = false,
            .gender = false,
            .nature = true,
        }
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        & NOTIFICATION_PROGRAM_FINISH,
        & NOTIFICATION_ERROR_FATAL,
    })
{
    {
        std::vector<std::unique_ptr<EditableTableRow>> ret;
        {
            auto row = std::make_unique<StatsHuntIvJudgeFilterRow>(FILTERS);
            row->iv_atk.set(IvJudgeFilter::NoGood);
            ret.emplace_back(std::move(row));
        }
        {
            auto row = std::make_unique<StatsHuntIvJudgeFilterRow>(FILTERS);
            row->iv_speed.set(IvJudgeFilter::NoGood);
            ret.emplace_back(std::move(row));
        }
        FILTERS.set_default(std::move(ret));
    }
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(LANGUAGE); //This is required
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(QUICKBALL);
    PA_ADD_OPTION(BATTLE_MOVES);
    PA_ADD_OPTION(FILTERS); //Note: None of these can be shiny, and the quartet will have some perfect IVs.
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool StatsReset::enter_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    //Press A to talk to target
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    pbf_press_button(context, BUTTON_A, 10, 50);
    int retD = wait_until(env.console, context, Milliseconds(4000), { advance_detector });
    if (retD < 0){
        env.log("Dialog not detected.");
    }

    switch (TARGET){
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
    case Target::Snacksworth:
        //The same as generic, but Snacksworth legendaries are not in the dex and skip the caught/summary/add to party menu.
        pbf_mash_button(context, BUTTON_B, 250);
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
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        env.log("Failed to enter battle!", COLOR_RED);

        return false;
        /*
        OperationFailedException::fire(
            env.console, ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter battle. Are you facing the Pokemon or in a menu?",
            true
        );
        */
    }
    return true;
}

void StatsReset::open_ball_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    BattleBallReader reader(env.console, LANGUAGE);
    std::string ball_reader = "";
    WallClock start = current_time();

    env.log("Opening ball menu...");
    while (ball_reader == ""){
        if (current_time() - start > std::chrono::minutes(2)){
            env.log("Timed out trying to read ball after 2 minutes.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Timed out trying to read ball after 2 minutes.",
                env.console
            );
        }

        //Mash B to exit anything else
        pbf_mash_button(context, BUTTON_B, 125);
        context.wait_for_all_requests();

        //Press X to open Ball menu
        pbf_press_button(context, BUTTON_X, 20, 100);
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        ball_reader = reader.read_ball(screen);
    }
}

//Returns target_fainted. If overworld is detected then the target fainted.
//Otherwise if AdvanceDialog is detected the Pokemon was caught or the player lost.
bool StatsReset::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    AdvanceDialogWatcher advance_dialog(COLOR_MAGENTA);
    OverworldWatcher overworld(env.console, COLOR_BLUE);

    uint8_t switch_party_slot = 1;

    size_t table_turn = 0;
    std::vector<std::unique_ptr<BattleMoveTableRow>> move_table = BATTLE_MOVES.copy_snapshot();

    bool target_fainted = false;
    bool out_of_balls = false;
    bool quickball_thrown = false;

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                //Check that battle menu appears - this is in case of swapping pokemon
                NormalBattleMenuWatcher menu_before_throw(COLOR_YELLOW);
                int bMenu = wait_until(
                    env.console, context,
                    std::chrono::seconds(15),
                    { menu_before_throw }
                );
                if (bMenu < 0){
                    env.console.log("Unable to find menu_before_throw.");
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Unable to find menu_before_throw.",
                        env.console
                    );
                }

                //Quick ball occurs before anything else in battle, so we can throw the ball without worrying about bounce/fainted/etc.
                if (QUICKBALL && !quickball_thrown){
                    env.log("Quick Ball option checked. Throwing Quick Ball.");

                    BattleBallReader reader(env.console, LANGUAGE);
                    open_ball_menu(env, context);

                    env.log("Selecting Quick Ball.");
                    int quantity = move_to_ball(reader, env.console, context, "quick-ball");
                    if (quantity == 0){
                        //Stop so user can check they have quick balls.
                        env.console.log("Unable to find Quick Ball on turn 1.");
                        stats.errors++;
                        env.update_stats();
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Unable to find Quick Ball on turn 1.",
                            env.console
                        );
                    }
                    if (quantity < 0){
                        stats.errors++;
                        env.update_stats();
                        env.console.log("Unable to read ball quantity.", COLOR_RED);
                    }

                    //Throw ball
                    env.log("Throwing Quick Ball.");
                    pbf_mash_button(context, BUTTON_A, 150);
                    context.wait_for_all_requests();

                    quickball_thrown = true;

                    stats.balls++;
                    env.update_stats();
                    pbf_mash_button(context, BUTTON_B, 900);
                    context.wait_for_all_requests();
                }else if (switch_party_slot == 1 && !move_table.empty() && table_turn < move_table.size()){
                    //Lead pokemon not fainted and table has not been completed
                    //Run through moves in table
                    env.log("Lead has not fainted, using move.");

                    MoveSelectWatcher move_watcher(COLOR_BLUE);
                    MoveSelectDetector move_select(COLOR_BLUE);
                    BattleMoveType move = move_table.at(table_turn)->type;
                    uint8_t move_slot = 0;

                    //Leaving room to expand to other battle actions later
                    switch (move){
                    case BattleMoveType::Move1:
                        move_slot = 0;
                        break;
                    case BattleMoveType::Move2:
                        move_slot = 1;
                        break;
                    case BattleMoveType::Move3:
                        move_slot = 2;
                        break;
                    case BattleMoveType::Move4:
                        move_slot = 3;
                        break;
                    }

                    //Select and use move
                    int ret_move_select = run_until<ProControllerContext>(
                    env.console, context,
                    [&](ProControllerContext& context){
                        pbf_press_button(context, BUTTON_A, 10, 50);
                        pbf_wait(context, 100);
                        context.wait_for_all_requests();
                    },
                    { move_watcher }
                    );
                    if (ret_move_select != 0){
                        env.log("Could not find move select.");
                    }else{
                        env.log("Move select found!");
                    }

                    context.wait_for_all_requests();
                    move_select.move_to_slot(env.console, context, move_slot);
                    pbf_mash_button(context, BUTTON_A, 150);
                    pbf_wait(context, 100);
                    context.wait_for_all_requests();
                    table_turn++;

                    //Check for battle menu
                    //If found after a second, assume out of PP and stop as this is a setup issue
                    //None of the target pokemon for this program have disable, taunt, etc.
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    int ret = wait_until(
                        env.console, context,
                        std::chrono::seconds(4),
                        { battle_menu }
                    );
                    if (ret == 0){
                        env.console.log("Battle menu detected early. Out of PP, please check your setup.");
                        stats.errors++;
                        env.update_stats();
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Battle menu detected early. Out of PP, please check your setup.",
                            env.console
                        );
                    }else{
                        env.log("Move successfully used.");
                        if (table_turn == move_table.size()){
                            env.log("End of table reached. Switch to throwing balls.");
                        }
                    }
                }else{
                    BattleBallReader reader(env.console, LANGUAGE);
                    open_ball_menu(env, context);

                    env.log("Selecting ball.");
                    int quantity = move_to_ball(reader, env.console, context, BALL_SELECT.slug());
                    if (quantity == 0){
                        out_of_balls = true;
                        env.console.log("Unable to find appropriate ball/out of balls.");
                        send_program_status_notification(
                            env, NOTIFICATION_STATUS_UPDATE,
                            "Unable to find appropriate ball/out of balls."
                        );
                        break;
                    }
                    if (quantity < 0){
                        stats.errors++;
                        env.update_stats();
                        env.console.log("Unable to read ball quantity.", COLOR_RED);
                    }

                    //Throw ball
                    env.log("Throwing selected ball.");
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
                    if (ret == 0){
                        env.console.log("Battle menu detected early. Using first attack.");
                        pbf_mash_button(context, BUTTON_A, 250);
                        context.wait_for_all_requests();
                    }else{
                        //Wild pokemon's turn/wait for catch animation
                        stats.balls++;
                        env.update_stats();
                        pbf_mash_button(context, BUTTON_B, 900);
                        context.wait_for_all_requests();
                    }
                }

                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                OverworldWatcher overworld(env.console, COLOR_BLUE);
                SwapMenuWatcher fainted(COLOR_YELLOW);
                int ret2 = wait_until(
                    env.console, context,
                    std::chrono::seconds(60),
                    { battle_menu, fainted }
                );
                switch (ret2){
                case 0:
                    env.log("Battle menu detected, continuing.");
                    break;
                case 1:
                    env.log("Detected fainted Pokemon. Switching to next living Pokemon...");
                    if (fainted.move_to_slot(env.console, context, switch_party_slot)){
                        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                        context.wait_for_all_requests();
                        switch_party_slot++;
                    }
                    break;
                default:
                    env.console.log("Invalid state ret2 run_battle.");
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Invalid state ret2 run_battle.",
                        env.console
                    );
                }

            }
        },
        { advance_dialog, overworld }
        );

    switch (ret){
    case 0:
        //Non-Snack: dialog appears on caught screen.
        //Snack: "target fled somewhere..."
        //Lost battle: joy dialog
        env.log("Advance Dialog detected. Caught regular target, lost battle, or fainted Snacksworth.");
        target_fainted = false;
        break;
    case 1:
        if (TARGET == Target::Snacksworth){
            env.log("Overworld detected. Snacksworth legendary caught, checking box system.");
            target_fainted = false;
        }else{
            env.log("Overworld detected, target Pokemon fainted.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Overworld detected, target Pokemon fainted."
            );
            target_fainted = true;
        }
        break;
    default:
        if (out_of_balls){
            target_fainted = true; //Resets game.
            env.log("Ran out of selected Pokeball. Resetting.");
            break;
        }
        env.console.log("Invalid state in run_battle().");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Invalid state in run_battle().",
            env.console
        );
    }

    return target_fainted;
}

bool StatsReset::check_stats(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();
    bool match = false;

    //  Open box
    enter_box_system_from_overworld(env.program_info(), env.console, context);
    context.wait_for(std::chrono::milliseconds(400));

    //  Check that the target pokemon was caught
    if (check_empty_slots_in_party(env.program_info(), env.console, context) != 0){
        env.console.log("One or more empty slots in party. Target was not caught or user setup error.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "One or more empty slots in party. Target was not caught."
        );
    }else{
        stats.catches++;
        env.update_stats();

        //  Navigate to last party slot
        move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 5, 0);

        //  Check the IVs of the newly caught Pokemon - *must be on IV panel*
        StatsHuntAction action = StatsHuntAction::Keep;
        check_stats_reset_info(env.console, context, LANGUAGE, FILTERS, action);

        switch (action){
        case StatsHuntAction::StopProgram:
            match = true;

            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

            env.console.log("Match found!");
            stats.matches++;
            env.update_stats();
            break;
        case StatsHuntAction::Discard:
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
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid state.",
                env.console
            );
        }
    }

    return match;
}

void StatsReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  This will only work for Pokemon that you press A to talk to.
    //  Regular static spawns will have the same stats, resetting won't work.
    //  Won't apply to the former titan pokemon or the box legends + ogrepon either, as their IVs are locked.
    //  So this really only applies to the ruinous quartet and loyal three
    //  Use first attack if target pokemon is invulnerable (Chi-Yu used Bounce)

    assert_16_9_720p_min(env.logger(), env.console);
    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 10);

    //  Autosave must be off, settings like Tera farmer.
    bool stats_matched = false;
    while (!stats_matched){
        bool battle_started = false;
        for (size_t c = 0; !battle_started; c++){
            battle_started = enter_battle(env, context);

            if (!battle_started){
                env.log("Did not detect battle. Resetting.");
                stats.resets++;
                env.update_stats();
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
                reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
            }

            //  Try to start battle 3 times.
            if (c > 2){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to enter battle after 3 attempts.",
                    env.console
                );
                break;
            }
        }
        
        bool target_fainted = run_battle(env, context);

        if (!target_fainted){
            //  Close all the dex entry and caught menus
            //  If the player lost, this closes all dialog from Joy
            OverworldWatcher overworld(env.console);
            int retOver = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 10000);
                },
                { overworld }
                );
            if (retOver != 0){
                env.log("Failed to detect overworld.", COLOR_RED);
            }else{
                env.log("Detected overworld.");
            }
            context.wait_for_all_requests();

            stats_matched = check_stats(env, context);
        }

        if (target_fainted || !stats_matched){
            //  Reset game
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Resetting game."
            );
            stats.resets++;
            env.update_stats();
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
            reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
        }
    }
    env.update_stats();
    auto screenshot = env.console.video().snapshot();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Match found!", screenshot, true
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}
    
}
}
}
