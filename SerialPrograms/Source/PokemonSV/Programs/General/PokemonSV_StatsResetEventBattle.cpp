/*  Stats Reset Event Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/PokemonSV_StatHexagonReader.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_StatsResetChecker.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV_StatsResetEventBattle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



IvDisplay::IvDisplay()
    : GroupOption("Calculated IV Range", LockMode::READ_ONLY)
    , hp(false, "<b>HP:</b>", LockMode::READ_ONLY, "-", "")
    , atk(false, "<b>Attack:</b>", LockMode::READ_ONLY, "-", "")
    , def(false, "<b>Defense:</b>", LockMode::READ_ONLY, "-", "")
    , spatk(false, "<b>Special Attack:</b>", LockMode::READ_ONLY, "-", "")
    , spdef(false, "<b>Special Defense:</b>", LockMode::READ_ONLY, "-", "")
    , speed(false, "<b>Speed:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_STATIC(hp);
    PA_ADD_STATIC(atk);
    PA_ADD_STATIC(def);
    PA_ADD_STATIC(spatk);
    PA_ADD_STATIC(spdef);
    PA_ADD_STATIC(speed);
}
std::string IvDisplay::get_range_string(const IvRange& range){
    if (range.low < 0 || range.high < 0){
        return "(invalid or unable to read)";
    }
    if (range.low == range.high){
        return std::to_string(range.low);
    }
    return std::to_string(range.low) + " - " + std::to_string(range.high);
}
void IvDisplay::set(const IvRanges& ivs){
    hp.set(get_range_string(ivs.hp));
    atk.set(get_range_string(ivs.attack));
    def.set(get_range_string(ivs.defense));
    spatk.set(get_range_string(ivs.spatk));
    spdef.set(get_range_string(ivs.spdef));
    speed.set(get_range_string(ivs.speed));
}


StatsResetEventBattle_Descriptor::StatsResetEventBattle_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:StatsResetEventBattle",
        STRING_POKEMON + " SV", "Stats Reset - Event Battle",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/StatsResetEventBattle.md",
        "Repeatedly catch Bloodmoon Ursaluna or Pecharunt until you get the stats you want.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StatsResetEventBattle_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , catches(m_stats["Catches"])
        , matches(m_stats["Matches"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Catches");
        m_display_order.emplace_back("Matches");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& catches;
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StatsResetEventBattle_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}
StatsResetEventBattle::StatsResetEventBattle()
    : TARGET(
        "<b>Target:</b><br>The Pokemon you are resetting for.",
        {
            {Target::Ursaluna, "ursaluna", "Bloodmoon Ursaluna"},
            {Target::Pecharunt, "pecharunt", "Pecharunt"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::Ursaluna
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
    , TRY_TO_TERASTILLIZE(
        "<b>Use Terastillization:</b><br>Tera at the start of battle.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
#if 0
    , FILTERS(
        StatsHuntIvJudgeFilterTable_Label_Regular,
        {
            .action = false,
            .shiny = false,
            .gender = false,
            .nature = false,
        }
    )
#endif
    , FILTERS0(
        StatsHuntIvRangeFilterTable_Label_Regular,
        {
            .action = false,
            .shiny = false,
            .gender = false,
            .nature = false,
        }
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
#if 0
    {
        std::vector<std::unique_ptr<EditableTableRow>> ret;
        {
            auto row = std::make_unique<StatsHuntIvJudgeFilterRow>(FILTERS.feature_flags);
            row->iv_atk.set(IvJudgeFilter::NoGood);
            ret.emplace_back(std::move(row));
        }
        {
            auto row = std::make_unique<StatsHuntIvJudgeFilterRow>(FILTERS.feature_flags);
            row->iv_speed.set(IvJudgeFilter::NoGood);
            ret.emplace_back(std::move(row));
        }
        FILTERS.set_default(std::move(ret));
    }
#endif
    {
        std::vector<std::unique_ptr<EditableTableRow>> ret;
        {
            auto row = std::make_unique<StatsHuntIvRangeFilterRow>(FILTERS0);
            row->iv_atk.set(0, 1);
            ret.emplace_back(std::move(row));
        }
        {
            auto row = std::make_unique<StatsHuntIvRangeFilterRow>(FILTERS0);
            row->iv_speed.set(0, 1);
            ret.emplace_back(std::move(row));
        }
        FILTERS0.set_default(std::move(ret));
        FILTERS0.restore_defaults();
    }
    PA_ADD_STATIC(CALCULATED_IVS);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(TRY_TO_TERASTILLIZE);
    PA_ADD_OPTION(FILTERS0);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StatsResetEventBattle::enter_battle_ursaluna(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    PromptDialogWatcher prompt_detector(COLOR_YELLOW);
    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);

    //  Initiate dialog with Perrin
    pbf_press_button(context, BUTTON_A, 10, 50);
    int ret = wait_until(env.console, context, Milliseconds(4000), { advance_detector });
    if (ret == 0){
        env.log("Dialog detected.");
    }else{
        env.log("Dialog not detected.");
    }
    //  Yes, ready
    pbf_mash_button(context, BUTTON_A, 400);
    context.wait_for_all_requests();

    //  Mash B until next dialog select
    int retPrompt = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10000);
        },
        { prompt_detector }
        );
    if (retPrompt != 0){
        env.log("Failed to detect prompt dialog!", COLOR_RED);
    }else{
        env.log("Detected prompt dialog.");
    }
    context.wait_for_all_requests();

    //  Pick an option to continue
    pbf_mash_button(context, BUTTON_A, 400);
    context.wait_for_all_requests();

    //  Mash B until next dialog select (again)
//    PromptDialogWatcher prompt_detector2(COLOR_YELLOW);
    int retPrompt2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10000);
        },
        { prompt_detector }
    );
    if (retPrompt2 != 0){
        env.log("Failed to detect prompt (again) dialog!", COLOR_RED);
    }else{
        env.log("Detected prompt (again) dialog.");
    }
    context.wait_for_all_requests();

    //  Pick an option to continue
    pbf_mash_button(context, BUTTON_A, 400);
    context.wait_for_all_requests();

    //  Now keep going until the battle starts
    int ret_battle = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10000);
        },
        { battle_menu }
    );
    if (ret_battle != 0){
        env.log("Failed to detect battle start!", COLOR_RED);
    }else{
        env.log("Battle started.");
    }
    context.wait_for_all_requests();
}

void StatsResetEventBattle::enter_battle_pecharunt(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    AdvanceDialogWatcher advance_detector2(COLOR_YELLOW);
    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);

    //  Talk to Pecharunt
    pbf_press_button(context, BUTTON_A, 10, 50);
    int ret = wait_until(env.console, context, Milliseconds(6000), { advance_detector });
    if (ret == 0){
        env.log("Dialog detected.");
    }else{
        env.log("Dialog not detected.");
    }
    pbf_mash_button(context, BUTTON_A, 400);
    context.wait_for_all_requests();

    //  Do you want to challenge the strange pokemon?
    int ret2 = wait_until(env.console, context, Milliseconds(6000), { advance_detector2 });
    if (ret2 == 0){
        env.log("Dialog detected.");
    }else{
        env.log("Dialog not detected.");
    }
    //  Mash through, answer Yes.
    pbf_mash_button(context, BUTTON_A, 500);
    context.wait_for_all_requests();

    //  Mash B until the battle starts
    //  Note - Sending out Ogerpon/Loyal Three during battle adds time, but the below is more than enough.
    int ret_battle = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10000);
        },
        { battle_menu }
        );
    if (ret_battle != 0){
        env.log("Failed to detect battle start!", COLOR_RED);
    }else{
        env.log("Battle started.");
    }
    context.wait_for_all_requests();
}

bool StatsResetEventBattle::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StatsResetEventBattle_Descriptor::Stats& stats = env.current_stats<StatsResetEventBattle_Descriptor::Stats>();

    //  Assuming the player has a charged orb
    if (TRY_TO_TERASTILLIZE){
        env.log("Attempting to terastillize.");
        //Open move menu
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        pbf_press_button(context, BUTTON_R, 20, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
    }

    //  Repeatedly use first attack
    TeraCatchWatcher catch_menu(COLOR_BLUE);
    AdvanceDialogWatcher lost(COLOR_YELLOW);
    WallClock start = current_time();
    uint8_t switch_party_slot = 1;

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while(true){
                if (current_time() - start > std::chrono::minutes(5)){
                    env.log("Timed out during battle after 5 minutes.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle after 5 minutes.",
                        env.console
                    );
                }

                NormalBattleMenuWatcher battle_menu(COLOR_MAGENTA);
                SwapMenuWatcher fainted(COLOR_RED);
                context.wait_for_all_requests();

                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(90),
                    { battle_menu, fainted }
                );
                switch (ret){
                case 0:
                    env.log("Detected battle menu. Pressing A to attack...");
                    pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
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
                    env.log("Timed out during battle. Stuck, crashed, or took more than 90 seconds for a turn.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle. Stuck, crashed, or took more than 90 seconds for a turn.",
                        env.console
                    );
                }
            }
        },
        { catch_menu, lost }
    );
    if (ret == 0){
        env.log("Catch prompt detected.");

        pbf_press_button(context, BUTTON_A, 20, 150);
        context.wait_for_all_requests();

        BattleBallReader reader(env.console, LANGUAGE);
        int quantity = move_to_ball(reader, env.console, context, BALL_SELECT.slug());
        if (quantity == 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to find appropriate ball. Did you run out?",
                env.console
            );
        }
        if (quantity < 0){
            stats.errors++;
            env.update_stats();
            env.log("Unable to read ball quantity.", COLOR_RED);
        }
        pbf_mash_button(context, BUTTON_A, 125);
        context.wait_for_all_requests();

        stats.catches++;
        env.update_stats();
        env.log("Target caught.");
    }else{
        env.log("Battle against target lost.", COLOR_RED);
        env.update_stats();
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Battle against target lost."
        );

        return false;
    }
    return true;
}


bool StatsResetEventBattle::check_stats_after_win(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  If this is the first advance dialog, it might be linked to pokedex filling so press A and continue
    bool first_advance_dialog = true;

    while (true){
        PromptDialogWatcher add_to_party(COLOR_PURPLE, {0.500, 0.395, 0.400, 0.100});
        PromptDialogWatcher view_summary(COLOR_PURPLE, {0.500, 0.470, 0.400, 0.100});
        PromptDialogWatcher nickname(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});
        PokemonSummaryWatcher summary(COLOR_MAGENTA);
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context, std::chrono::seconds(60),
            {
                add_to_party,
                view_summary,
                nickname,
                summary,
                advance,
            }
        );
        switch (ret){
        case 0:
            env.console.log("Detected add-to-party prompt.");
            pbf_press_dpad(context, DPAD_DOWN, 20, 60);
            continue;
        case 1:
            env.console.log("Detected cursor over view summary.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:
            env.console.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:{
            env.console.log("Detected summary.");
            VideoOverlaySet overlays(env.console.overlay());

            SummaryStatsReader reader;
            reader.make_overlays(overlays);

            pbf_press_dpad(context, DPAD_RIGHT, 20, 230);
            context.wait_for_all_requests();

            auto snapshot = env.console.video().snapshot();
            IvRanges ivs;
            try{
                if (TARGET == Target::Ursaluna){
                    ivs = reader.calc_ivs(env.logger(), snapshot, { 113, 70, 120, 135, 65, 52 });
                }else{
                    ivs = reader.calc_ivs(env.logger(), snapshot, { 88, 88, 160, 88, 88, 88 });
                }
            }catch (OperationFailedException& e){
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    e.message(),
                    snapshot
                );
                return false;
            }

            CALCULATED_IVS.set(ivs);

            StatsHuntAction action;
            if (TARGET == Target::Ursaluna){
                action = FILTERS0.get_action(false, StatsHuntGenderFilter::Any, NatureCheckerValue::Hardy, ivs);
            }else{
                action = FILTERS0.get_action(false, StatsHuntGenderFilter::Any, NatureCheckerValue::Timid, ivs);
            }

            return action != StatsHuntAction::Discard;
        }
        case 4:{
            if (first_advance_dialog){
                env.console.log("Pressing continue, in case it's a new pokedex entry.");
                pbf_press_button(context, BUTTON_A, 20, 105);
                first_advance_dialog = false;
                continue;
            }else{
                StatsResetEventBattle_Descriptor::Stats& stats = env.current_stats<StatsResetEventBattle_Descriptor::Stats>();
                stats.errors++;
                env.update_stats();
                throw UserSetupError(
                    env.logger(),
                    "Did not detect add-to-party prompt. Make sure your party is full and \"Send to Boxes\" is set to \"Manual\"."
                );
            }
        }
        default:
            StatsResetEventBattle_Descriptor::Stats& stats = env.current_stats<StatsResetEventBattle_Descriptor::Stats>();
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "StatsResetEventBattle::check_stats_after_win(): No state detected after 1 minute.",
                env.console
            );
        }
    }
}

void StatsResetEventBattle::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    StatsResetEventBattle_Descriptor::Stats& stats = env.current_stats<StatsResetEventBattle_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 10);

    while (true){
        if (TARGET == Target::Ursaluna){
            enter_battle_ursaluna(env, context);
        }else{
            enter_battle_pecharunt(env, context);
        }

        if (run_battle(env, context) && check_stats_after_win(env, context)){
            break;
        }

        //  Reset
        stats.resets++;
        env.update_stats();
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Resetting game."
        );
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
    }
    stats.matches++;
    env.update_stats();

    auto snapshot = env.console.video().snapshot();

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "HP: " + (std::string)CALCULATED_IVS.hp + "\n" +
        "Atk: " + (std::string)CALCULATED_IVS.atk + "\n" +
        "Def: " + (std::string)CALCULATED_IVS.def + "\n" +
        "SpAtk: " + (std::string)CALCULATED_IVS.spatk + "\n" +
        "SpDef: " + (std::string)CALCULATED_IVS.spdef + "\n" +
        "Speed: " + (std::string)CALCULATED_IVS.speed,
        snapshot,
        true
    );
}

}
}
}
