/*  Money Farmer (Route 210)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GlobalRoomHeal.h"
#include "PokemonBDSP/Inference/PokemonBDSP_VSSeekerReaction.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP_MoneyFarmerRoute210.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


MoneyFarmerRoute210_Descriptor::MoneyFarmerRoute210_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:MoneyFarmerRoute210",
        STRING_POKEMON + " BDSP", "Money Farmer (Route 210)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/MoneyFarmerRoute210.md",
        "Farm money by using VS Seeker to rebattle the Ace Trainer couple on Route 210.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct MoneyFarmerRoute210_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_searches(m_stats["Searches"])
        , m_errors(m_stats["Errors"])
        , m_noreact(m_stats["No React"])
        , m_react(m_stats["React"])
    {
        m_display_order.emplace_back("Searches");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("No React");
        m_display_order.emplace_back("React");
    }
    std::atomic<uint64_t>& m_searches;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_noreact;
    std::atomic<uint64_t>& m_react;
};
std::unique_ptr<StatsTracker> MoneyFarmerRoute210_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MoneyFarmerRoute210::MoneyFarmerRoute210()
    : SHORTCUT("<b>VS Seeker Shortcut:</b>")
    , START_LOCATION(
        "<b>Start Location:</b>",
        {
            {StartLocation::CelesticTown, "celestic", "In front of the Celestic Town " + STRING_POKEMON + " center."},
            {StartLocation::AceTrainerPair, "trainer-pair", "Lower-most row of the platform the Ace Trainer pair in Route 210 is on."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartLocation::CelesticTown
    )
    , HEALING_METHOD(
        "<b> Healing method:</b>",
        {
            {HealMethod::CelesticTown, "celestic", "Celestic Town " + STRING_POKEMON + " center."},
            {HealMethod::GlobalRoom, "global-room", "Use Global Room. (will force update your game)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HealMethod::CelesticTown
    )
    , MON0_MOVE1_PP("<b>Lead " + STRING_POKEMON + " Move 1 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON0_MOVE2_PP("<b>Lead " + STRING_POKEMON + " Move 2 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON0_MOVE3_PP("<b>Lead " + STRING_POKEMON + " Move 3 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON0_MOVE4_PP("<b>Lead " + STRING_POKEMON + " Move 4 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON1_MOVE1_PP("<b>2nd " + STRING_POKEMON + " Move 1 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON1_MOVE2_PP("<b>2nd " + STRING_POKEMON + " Move 2 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON1_MOVE3_PP("<b>2nd " + STRING_POKEMON + " Move 3 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MON1_MOVE4_PP("<b>2nd " + STRING_POKEMON + " Move 4 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(HEALING_METHOD);
    PA_ADD_OPTION(ON_LEARN_MOVE);
    PA_ADD_OPTION(MON0_MOVE1_PP);
    PA_ADD_OPTION(MON0_MOVE2_PP);
    PA_ADD_OPTION(MON0_MOVE3_PP);
    PA_ADD_OPTION(MON0_MOVE4_PP);
    PA_ADD_OPTION(MON1_MOVE1_PP);
    PA_ADD_OPTION(MON1_MOVE2_PP);
    PA_ADD_OPTION(MON1_MOVE3_PP);
    PA_ADD_OPTION(MON1_MOVE4_PP);
    PA_ADD_OPTION(NOTIFICATIONS);
}




bool MoneyFarmerRoute210::battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t pp0[4], uint8_t pp1[4]){
    MoneyFarmerRoute210_Descriptor::Stats& stats = env.current_stats<MoneyFarmerRoute210_Descriptor::Stats>();

    env.log("Starting battle!");

    {
        StartBattleDetector detector(env.console);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_ZL, 10, 10);
                for (size_t c = 0; c < 17; c++){
                    pbf_press_dpad(context, DPAD_UP, 5, 10);
                    pbf_press_button(context, BUTTON_ZL, 10, 10);
                    pbf_press_dpad(context, DPAD_RIGHT, 20, 10);
                    pbf_press_button(context, BUTTON_ZL, 10, 10);
                }
            },
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            env.log("Failed to detect start of battle after 20 seconds.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
            return false;
        }
    }
    pbf_wait(context, 5 * TICKS_PER_SECOND);

    bool battle_menu_seen = false;

    //  State Machine
    //  We need lots of loops in case the party pokemon need to learn lots of moves.
    while (true){
        context.wait_for_all_requests();

        BattleMenuWatcher battle_menu(BattleType::TRAINER);
        EndBattleWatcher end_battle;
        SelectionArrowFinder learn_move(env.console, {0.50, 0.62, 0.40, 0.18}, COLOR_YELLOW);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {
                {battle_menu},
                battle_menu_seen ? PeriodicInferenceCallback{end_battle} : PeriodicInferenceCallback{},
                {learn_move},
            }
        );
        switch (ret){
        case 0:        
            env.log("Battle menu detected!", COLOR_BLUE);
            battle_menu_seen = true;

            {
                pbf_press_button(context, BUTTON_ZL, 10, 125);
                uint8_t slot = 0;
                for (; slot < 4; slot++){
                    if (pp0[slot] != 0){
                        break;
                    }
                }
                if (slot == 4){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Ran out of PP in a battle.",
                        env.console
                    );
                }

                for (uint8_t move_slot = 0; move_slot < slot; move_slot++){
                    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
                }
                pbf_press_button(context, BUTTON_ZL, 10, 125);
                pbf_press_button(context, BUTTON_ZL, 10, 375);
                pp0[slot]--;
            }

            {
                pbf_press_button(context, BUTTON_ZL, 10, 125);
                uint8_t slot = 0;
                for (; slot < 4; slot++){
                    if (pp1[slot] != 0){
                        break;
                    }
                }
                if (slot == 4){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Ran out of PP in a battle.",
                        env.console
                    );
                }

                for (uint8_t move_slot = 0; move_slot < slot; move_slot++){
                    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
                }
                pbf_press_button(context, BUTTON_ZL, 10, 125);
                pbf_press_button(context, BUTTON_ZL, 10, 375);
                pp1[slot]--;
            }

            break;
        case 1:
            env.log("Battle finished!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 250);
            return false;
        case 2:
            env.log("Detected move learn!", COLOR_BLUE);
            if (ON_LEARN_MOVE == OnLearnMove::DONT_LEARN){
                pbf_move_right_joystick(context, 128, 255, 20, 105);
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;
            }
            return true;
        default:
            stats.m_errors++;
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Timed out after 2 minutes.",
                env.console
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "No progress detected after 5 battle menus. Are you out of PP?",
        env.console
    );
}

void MoneyFarmerRoute210::heal_at_center_and_return(
    Logger& logger, ProControllerContext& context,
    uint8_t pp0[4], uint8_t pp1[4]
){
    logger.log("Healing " + STRING_POKEMON + " Celestic Town " + STRING_POKEMON + " Center.");
    pbf_move_left_joystick(context, 125, 0, 6 * TICKS_PER_SECOND, 0);
    pbf_mash_button(context, BUTTON_ZL, 3 * TICKS_PER_SECOND);
    pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);

    logger.log("Returning to trainers...");
    pbf_move_left_joystick(context, 128, 255, 6 * TICKS_PER_SECOND, 0);
    pbf_move_left_joystick(context, 255, 128, 60, 0);
    pbf_move_left_joystick(context, 128, 0, 200, 0);
    pbf_move_left_joystick(context, 255, 128, 750, 0);

    pbf_press_button(context, BUTTON_R, 10, 150);
    pbf_mash_button(context, BUTTON_ZL, 6 * TICKS_PER_SECOND);

    pbf_move_left_joystick(context, 128, 255, 30, 0);
    pbf_move_left_joystick(context,   0, 128, 30, 0);
    pbf_move_left_joystick(context, 128, 255, 80, 0);
    pbf_move_left_joystick(context, 255, 128, 110, 0);
    pbf_move_left_joystick(context, 128, 255, 125, 0);
    pbf_move_left_joystick(context, 255, 128, 105, 0);
    pbf_move_left_joystick(context, 128,   0, 375, 0);
    pbf_move_left_joystick(context, 255, 128, 300, 0);
    pbf_move_left_joystick(context, 128, 255, 375, 0);

    pbf_press_dpad(context, DPAD_RIGHT, 375, 0);
    pbf_press_dpad(context, DPAD_LEFT, 375, 0);
    pbf_press_dpad(context, DPAD_DOWN, 125, 0);

    pp0[0] = MON0_MOVE1_PP;
    pp0[1] = MON0_MOVE2_PP;
    pp0[2] = MON0_MOVE3_PP;
    pp0[3] = MON0_MOVE4_PP;
    pp1[0] = MON1_MOVE1_PP;
    pp1[1] = MON1_MOVE2_PP;
    pp1[2] = MON1_MOVE3_PP;
    pp1[3] = MON1_MOVE4_PP;
}
void MoneyFarmerRoute210::fly_to_center_heal_and_return(
    Logger& logger, ProControllerContext& context,
    uint8_t pp0[4], uint8_t pp1[4]
){
    logger.log("Flying back to Hearthome City to heal.");
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_PLUS, 10, 240);
    pbf_press_dpad(context, DPAD_LEFT, 10, 60);
    pbf_press_dpad(context, DPAD_LEFT, 10, 60);
    pbf_mash_button(context, BUTTON_ZL, 12 * TICKS_PER_SECOND);
    heal_at_center_and_return(logger, context, pp0, pp1);
}

bool MoneyFarmerRoute210::heal_after_battle_and_return(
    SingleSwitchProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    uint8_t pp0[4], uint8_t pp1[4])
{
    if (HEALING_METHOD == HealMethod::CelesticTown){
        // Go to Celestic Town Pokecenter to heal the party.
        fly_to_center_heal_and_return(stream.logger(), context, pp0, pp1);
        return false;
    }else{
        // Use Global Room to heal the party.
        heal_by_global_room(stream, context);

        pp0[0] = MON0_MOVE1_PP;
        pp0[1] = MON0_MOVE2_PP;
        pp0[2] = MON0_MOVE3_PP;
        pp0[3] = MON0_MOVE4_PP;
        pp1[0] = MON1_MOVE1_PP;
        pp1[1] = MON1_MOVE2_PP;
        pp1[2] = MON1_MOVE3_PP;
        pp1[3] = MON1_MOVE4_PP;
        return true;
    }
}


bool MoneyFarmerRoute210::has_pp(uint8_t pp0[4], uint8_t pp1[4]){
    size_t count0 = 0;
    size_t count1 = 0;
    for (size_t c = 0; c < 4; c++){
        count0 += pp0[c];
        count1 += pp1[c];
    }
    return count0 > 0 && count1 > 0;
}



void MoneyFarmerRoute210::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    MoneyFarmerRoute210_Descriptor::Stats& stats = env.current_stats<MoneyFarmerRoute210_Descriptor::Stats>();

    uint8_t pp0[4] = {
        MON0_MOVE1_PP,
        MON0_MOVE2_PP,
        MON0_MOVE3_PP,
        MON0_MOVE4_PP,
    };
    uint8_t pp1[4] = {
        MON1_MOVE1_PP,
        MON1_MOVE2_PP,
        MON1_MOVE3_PP,
        MON1_MOVE4_PP,
    };

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    bool need_to_charge = true;
    if (START_LOCATION == StartLocation::CelesticTown){
        heal_at_center_and_return(env.console, context, pp0, pp1);
        need_to_charge = false;
    }else{
        if (HEALING_METHOD == HealMethod::GlobalRoom){
            heal_by_global_room(env.console, context);
        }
        pbf_move_left_joystick(context, 255, 128, 140, 0);
    }

    while (true){
        env.update_stats();

        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (need_to_charge){
            pbf_move_left_joystick(context, 255, 128, 140, 0);
            pbf_press_dpad(context, DPAD_UP, 85, 0);
            for (size_t c = 0; c < 7; c++){
                pbf_move_left_joystick(context, 0, 128, 140, 0);
                pbf_move_left_joystick(context, 255, 128, 140, 0);
            }
            pbf_press_dpad(context, DPAD_DOWN, 75, 0);
        }
        pbf_press_dpad(context, DPAD_LEFT, 200, 0);

        context.wait_for_all_requests();
        stats.m_searches++;

        std::vector<ImagePixelBox> bubbles;
        {
            VSSeekerReactionTracker tracker(env.console, {0.20, 0.20, 0.60, 0.60});
            run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    SHORTCUT.run(context, TICKS_PER_SECOND);
                },
                {{tracker}}
            );
            need_to_charge = true;
            pbf_mash_button(context, BUTTON_B, 250);

            bubbles = tracker.reactions();
            if (bubbles.empty()){
                env.log("No reactions.", COLOR_ORANGE);
                stats.m_noreact++;
                continue;
            }
            stats.m_react++;
        }
        for (const ImagePixelBox& box : bubbles){
            env.log("Reaction at: " + std::to_string(box.min_x), COLOR_BLUE);
        }

        if (this->battle(env, context, pp0, pp1)){
            return;
        }
        if (!has_pp(pp0, pp1)){
            need_to_charge = heal_after_battle_and_return(env, env.console, context, pp0, pp1);
            continue;
        }
    }
}






}
}
}










