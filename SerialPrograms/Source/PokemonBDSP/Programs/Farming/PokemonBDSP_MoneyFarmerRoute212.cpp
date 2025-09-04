/*  Money Farmer
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
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP_MoneyFarmerRoute212.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


MoneyFarmerRoute212_Descriptor::MoneyFarmerRoute212_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:MoneyFarmerRoute212",
        STRING_POKEMON + " BDSP", "Money Farmer (Route 212)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/MoneyFarmerRoute212.md",
        "Farm money by using VS Seeker to rebattle the rich couple on Route 212.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct MoneyFarmerRoute212_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_searches(m_stats["Searches"])
        , m_errors(m_stats["Errors"])
        , m_nothing(m_stats["No React"])
        , m_man(m_stats["Man Only"])
        , m_woman(m_stats["Woman Only"])
        , m_both(m_stats["Both"])
    {
        m_display_order.emplace_back("Searches");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("No React");
        m_display_order.emplace_back("Man Only");
        m_display_order.emplace_back("Woman Only");
        m_display_order.emplace_back("Both");
    }
    std::atomic<uint64_t>& m_searches;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_nothing;
    std::atomic<uint64_t>& m_man;
    std::atomic<uint64_t>& m_woman;
    std::atomic<uint64_t>& m_both;
};
std::unique_ptr<StatsTracker> MoneyFarmerRoute212_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


MoneyFarmerRoute212::MoneyFarmerRoute212()
    : SHORTCUT("<b>VS Seeker Shortcut:</b>")
    , START_LOCATION(
        "<b>Start Location:</b>",
        {
            {StartLocation::Hearthome, "hearthome", "In front of the Hearthome City " + STRING_POKEMON + " center."},
            {StartLocation::OldCouple, "old-couple", "In the row above the rich couple in Route 212."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartLocation::Hearthome
    )
    , HEALING_METHOD(
        "<b> Healing method:</b>",
        {
            {HealMethod::Hearthome, "hearthome", "Hearthome City " + STRING_POKEMON + " center."},
            {HealMethod::GlobalRoom, "global-room", "Use Global Room. (will force update your game)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HealMethod::Hearthome
    )
    , MOVE1_PP("<b>Move 1 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MOVE2_PP("<b>Move 2 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MOVE3_PP("<b>Move 3 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
    , MOVE4_PP("<b>Move 4 PP:</b><br>Set to zero to not use this move.", LockMode::LOCK_WHILE_RUNNING, 5, 0, 64)
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
    PA_ADD_OPTION(MOVE1_PP);
    PA_ADD_OPTION(MOVE2_PP);
    PA_ADD_OPTION(MOVE3_PP);
    PA_ADD_OPTION(MOVE4_PP);
    PA_ADD_OPTION(NOTIFICATIONS);
}




bool MoneyFarmerRoute212::battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t pp[4], bool man){
    MoneyFarmerRoute212_Descriptor::Stats& stats = env.current_stats<MoneyFarmerRoute212_Descriptor::Stats>();
    context.wait_for_all_requests();
    if (man){
        env.log("Starting battle with man (left).");
    }else{
        env.log("Starting battle with woman (right).");
    }
    env.console.overlay().add_log("Starting battle", COLOR_WHITE);

    pbf_mash_button(context, BUTTON_ZL, 5 * TICKS_PER_SECOND);

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
                pbf_mash_button(context, BUTTON_B, 30 * TICKS_PER_SECOND);
            },
            {
                {battle_menu},
                battle_menu_seen ? PeriodicInferenceCallback{end_battle} : PeriodicInferenceCallback{},
                {learn_move},
            }
        );
        switch (ret){
        case 0:{
            env.log("Battle menu detected!", COLOR_BLUE);
            env.console.overlay().add_log("Choose move", COLOR_BLUE);
            battle_menu_seen = true;

            pbf_press_button(context, BUTTON_ZL, 10, 125);

            uint8_t slot = 0;
            for (; slot < 4; slot++){
                if (pp[slot] != 0){
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
            pbf_mash_button(context, BUTTON_ZL, 250);
            pp[slot]--;
            env.log("Used move at slot " + std::to_string(slot+1) + ". " + std::to_string(pp[slot]) + " PP left.", COLOR_BLUE);

            break;
        }
        case 1:
            env.log("Battle finished!", COLOR_BLUE);
            env.console.overlay().add_log("Battle finished", COLOR_WHITE);
            pbf_mash_button(context, BUTTON_B, 250);
            return false;
//        case 1:
//            env.log("Dialog detected! Battle finished?", COLOR_BLUE);
//            pbf_mash_button(context, BUTTON_B, 250);
//            return;
        case 2:
            env.log("Detected move to learn!", COLOR_BLUE);
            env.console.overlay().add_log("Detected move to learn", COLOR_WHITE);
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
                "Timed out after 30 seconds.",
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


void MoneyFarmerRoute212::heal_at_center_and_return(VideoStream& stream, ProControllerContext& context, uint8_t pp[4]){
    stream.overlay().add_log("Heal at " + STRING_POKEMON + " Center", COLOR_WHITE);
    stream.log("Healing " + STRING_POKEMON + " at Hearthome City " + STRING_POKEMON + " Center.");
    pbf_move_left_joystick(context, 125, 0, 6 * TICKS_PER_SECOND, 0);
    pbf_mash_button(context, BUTTON_ZL, 3 * TICKS_PER_SECOND);
//    ssf_mash_AZs(context, 3 * TICKS_PER_SECOND);
    pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    stream.overlay().add_log("Heal complete", COLOR_WHITE);
    stream.overlay().add_log("To rich couple", COLOR_WHITE);
    stream.log("Returning to rich couple location...");
    pbf_move_left_joystick(context, 128, 255, 8 * TICKS_PER_SECOND, 0);
    pbf_move_left_joystick(context, 255, 128, 380, 0);
    pbf_move_left_joystick(context, 128, 255, 300, 0);
    pbf_move_left_joystick(context,   0, 128, 600, 0);
    pbf_move_left_joystick(context, 255, 128,  75, 0);
    pbf_move_left_joystick(context, 128, 255, 1375, 0);
    pbf_move_left_joystick(context, 255, 128, 125, 0);
    pbf_move_left_joystick(context, 128, 255, 200, 0);
    pbf_move_left_joystick(context,   0, 128, 200, 0);
    pbf_move_left_joystick(context, 128, 255,  50, 0);
    pbf_move_left_joystick(context,   0, 128, 125, 0);
    pbf_move_left_joystick(context, 128, 255, 125, 0);
    pbf_move_left_joystick(context, 255, 128, 250, 0);
    pbf_move_left_joystick(context, 128, 255, 200, 0);
    pbf_move_left_joystick(context,   0, 128,  90, 0);
    pbf_move_left_joystick(context, 128, 255, 200, 0);
    pbf_move_left_joystick(context, 255, 128, 125, 0);
    pbf_move_left_joystick(context, 128, 255, 200, 0);

    pp[0] = MOVE1_PP;
    pp[1] = MOVE2_PP;
    pp[2] = MOVE3_PP;
    pp[3] = MOVE4_PP;
    context.wait_for_all_requests();
}


void MoneyFarmerRoute212::fly_to_center_heal_and_return(VideoStream& stream, ProControllerContext& context, uint8_t pp[4]){
    stream.log("Flying back to Hearthome City to heal.");
    stream.overlay().add_log("Fly to Hearthome City", COLOR_WHITE);
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_PLUS, 10, 240);
    pbf_press_dpad(context, DPAD_UP, 10, 60);
    pbf_press_dpad(context, DPAD_UP, 10, 60);
    pbf_mash_button(context, BUTTON_ZL, 12 * TICKS_PER_SECOND);
    heal_at_center_and_return(stream, context, pp);
}

bool MoneyFarmerRoute212::heal_after_battle_and_return(
    VideoStream& stream, ProControllerContext& context,
    uint8_t pp[4])
{
    if (HEALING_METHOD == HealMethod::Hearthome){
        // Go to Hearhome City Pokecenter to heal the party.
        fly_to_center_heal_and_return(stream, context, pp);
        return false;
    }else{
        // Use Global Room to heal the party.
        heal_by_global_room(stream, context);

        pp[0] = MOVE1_PP;
        pp[1] = MOVE2_PP;
        pp[2] = MOVE3_PP;
        pp[3] = MOVE4_PP;
        return true;
    }
}

void MoneyFarmerRoute212::charge_vs_seeker(ProControllerContext& context){
    for (size_t c = 0; c < 5; c++){
        pbf_move_left_joystick(context, 0, 128, 180, 0);
        pbf_move_left_joystick(context, 255, 128, 180, 0);
    }
}


size_t MoneyFarmerRoute212::total_pp(uint8_t pp[4]){
    size_t ret = 0;
    ret += pp[0];
    ret += pp[1];
    ret += pp[2];
    ret += pp[3];
    return ret;
}


void MoneyFarmerRoute212::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    MoneyFarmerRoute212_Descriptor::Stats& stats = env.current_stats<MoneyFarmerRoute212_Descriptor::Stats>();

    uint8_t pp[4] = {
        MOVE1_PP,
        MOVE2_PP,
        MOVE3_PP,
        MOVE4_PP,
    };

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    bool need_to_charge = true;
    if (START_LOCATION == StartLocation::Hearthome){
        heal_at_center_and_return(env.console, context, pp);
        need_to_charge = false;
    }else{ // start location is the row above the rich couple
        if (HEALING_METHOD == HealMethod::GlobalRoom){
            heal_by_global_room(env.console, context);
        }
        // Reset position to right most on the row above the rich couple
        pbf_move_left_joystick(context, 255, 128, 180, 0);
    }

    while (true){
        context.wait_for_all_requests();
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (need_to_charge){
            env.console.overlay().add_log("Charging Vs. Seeker", COLOR_WHITE);
            charge_vs_seeker(context);
        }

        //  Move to woman.
        pbf_move_left_joystick(context, 0, 128, 52, 0);

        context.wait_for_all_requests();
        stats.m_searches++;

        QSize dimensions;
        std::vector<ImagePixelBox> bubbles;
        {
            env.console.overlay().add_log("Using Vs. Seeker", COLOR_WHITE);
            VSSeekerReactionTracker tracker(env.console, {0.23, 0.30, 0.35, 0.30});
            run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    SHORTCUT.run(context, TICKS_PER_SECOND);

                },
                {{tracker}}
            );
            need_to_charge = true;
            pbf_mash_button(context, BUTTON_B, 250);

            dimensions = tracker.dimensions();
            bubbles = tracker.reactions();
            if (bubbles.empty()){
                env.log("No reactions.", COLOR_ORANGE);
                stats.m_nothing++;
                continue;
            }
        }

        bool man = false;
        bool woman = false;
        for (const ImagePixelBox& box : bubbles){
            double x_coord = (double)box.min_x / dimensions.width();
            env.log("Reaction at: " + std::to_string(x_coord), COLOR_BLUE);
            if (x_coord < 0.5){
                man = true;
            }
            if (x_coord > 0.5){
                woman = true;
            }
        }

        if (man && woman){
            env.console.overlay().add_log("Found both", COLOR_GREEN);
            stats.m_both++;
        }else if (man){
            env.console.overlay().add_log("Found Gentleman", COLOR_GREEN);
            stats.m_man++;
        }else if (woman){
            env.console.overlay().add_log("Found Madame", COLOR_GREEN);
            stats.m_woman++;
        }

        env.update_stats();

        if (woman){
//            pbf_move_left_joystick(context, 0, 128, 52, 0);
            pbf_move_left_joystick(context, 128, 255, 10, 0);

            //  Battle woman.
            if(battle(env, context, pp, false)){
                return;
            }

            //  Check PP.
            if (total_pp(pp) == 0){
                need_to_charge = heal_after_battle_and_return(env.console, context, pp);
                continue;
            }
        }
        if (man){
#if 0
            //  Make sure we have enough PP.
            if (total_pp(pp) < 2){
                need_to_charge = heal_after_battle_and_return(env.console, pp);
                continue;
            }
#endif

//            if (woman){
                pbf_move_left_joystick(context, 0, 128, 52, 0);
                pbf_move_left_joystick(context, 128, 255, 10, 0);
//            }else{
//                pbf_move_left_joystick(context, 0, 128, 105, 0);
//                pbf_move_left_joystick(context, 128, 255, 10, 0);
//            }

            //  Battle man.
            if (battle(env, context, pp, true)){
                return;
            }

            //  Check PP.
            if (total_pp(pp) == 0){
                need_to_charge = heal_after_battle_and_return(env.console, context, pp);
                continue;
            }
        }
        pbf_move_left_joystick(context, 255, 128, 180, 0);

    }
}







}
}
}










