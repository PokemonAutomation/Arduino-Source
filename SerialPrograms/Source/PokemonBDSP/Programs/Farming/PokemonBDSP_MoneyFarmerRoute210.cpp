/*  Money Farmer (Route 210)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_VSSeekerReaction.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP_MoneyFarmerRoute210.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


MoneyFarmerRoute210_Descriptor::MoneyFarmerRoute210_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:MoneyFarmerRoute210",
        STRING_POKEMON + " BDSP", "Money Farmer (Route 210)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/MoneyFarmerRoute210.md",
        "Farm money by using VS Seeker to rebattle the Ace Trainer couple on Route 210.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



MoneyFarmerRoute210::MoneyFarmerRoute210(const MoneyFarmerRoute210_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SHORTCUT("<b>VS Seeker Shortcut:</b>")
    , MON0_MOVE1_PP("<b>Lead " + STRING_POKEMON + " Move 1 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON0_MOVE2_PP("<b>Lead " + STRING_POKEMON + " Move 2 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON0_MOVE3_PP("<b>Lead " + STRING_POKEMON + " Move 3 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON0_MOVE4_PP("<b>Lead " + STRING_POKEMON + " Move 4 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON1_MOVE1_PP("<b>2nd " + STRING_POKEMON + " Move 1 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON1_MOVE2_PP("<b>2nd " + STRING_POKEMON + " Move 2 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON1_MOVE3_PP("<b>2nd " + STRING_POKEMON + " Move 3 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , MON1_MOVE4_PP("<b>2nd " + STRING_POKEMON + " Move 4 PP:</b><br>Set to zero to not use this move.", 5, 0, 64)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SHORTCUT);
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


struct MoneyFarmerRoute210::Stats : public StatsTracker{
    Stats()
        : m_searches(m_stats["Searches"])
        , m_errors(m_stats["Errors"])
        , m_noreact(m_stats["No React"])
        , m_react(m_stats["React"])
    {
        m_display_order.emplace_back("Searches");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("No React");
        m_display_order.emplace_back("React");
    }
    std::atomic<uint64_t>& m_searches;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_noreact;
    std::atomic<uint64_t>& m_react;
};
std::unique_ptr<StatsTracker> MoneyFarmerRoute210::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void MoneyFarmerRoute210::battle(SingleSwitchProgramEnvironment& env, uint8_t pp0[4], uint8_t pp1[4]){
    Stats& stats = env.stats<Stats>();

    env.log("Starting battle!");

    {
        StartBattleDetector detector(env.console);
        int ret = run_until(
            env, env.console,
            [=](const BotBaseContext& context){
                pbf_press_button(context, BUTTON_ZL, 10, 10);
                for (size_t c = 0; c < 17; c++){
                    pbf_press_dpad(context, DPAD_UP, 5, 10);
                    pbf_press_button(context, BUTTON_ZL, 10, 10);
                    pbf_press_dpad(context, DPAD_RIGHT, 20, 10);
                    pbf_press_button(context, BUTTON_ZL, 10, 10);
                }
            },
            { &detector, }
        );
        if (ret < 0){
            stats.m_errors++;
            env.log("Failed to detect start of battle after 20 seconds.", COLOR_RED);
            pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
            return;
        }
    }
    pbf_wait(env.console, 5 * TICKS_PER_SECOND);

    uint8_t move_slot0 = 0;
    uint8_t move_slot1 = 0;
    bool battle_menu_seen = false;

    //  State Machine
    for (size_t c = 0; c < 5; c++){
        env.console.botbase().wait_for_all_requests();

        BattleMenuWatcher battle_menu(BattleType::TRAINER);
        EndBattleWatcher end_battle;
//        ShortDialogDetectorCallback dialog_detector(env.console);
        int ret = run_until(
            env, env.console,
            [=](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {
                &battle_menu,
                battle_menu_seen ? &end_battle : nullptr,
//                &dialog_detector,
            }
        );
        switch (ret){
        case 0:
            break;
        case 1:
            env.log("Battle finished!", COLOR_BLUE);
            pbf_mash_button(env.console, BUTTON_B, 250);
            return;
        default:
            env.log("Timed out.", COLOR_RED);
            stats.m_errors++;
            PA_THROW_StringException("Timed out after 2 minutes.");
        }

        env.log("Battle menu detected!", COLOR_BLUE);
        battle_menu_seen = true;

        {
            pbf_press_button(env.console, BUTTON_ZL, 10, 125);
            uint8_t slot = 0;
            for (; slot < 4; slot++){
                if (pp0[slot] != 0){
                    break;
                }
            }
            if (slot == 4){
                env.log("Ran out of PP in a battle.", COLOR_RED);
                PA_THROW_StringException("Ran out of PP in a battle.");
            }

            while (move_slot0 < slot){
                move_slot0++;
                pbf_press_dpad(env.console, DPAD_DOWN, 10, 50);
            }
            pbf_press_button(env.console, BUTTON_ZL, 10, 125);
            pbf_press_button(env.console, BUTTON_ZL, 10, 375);
            pp0[slot]--;
        }
        {
            pbf_press_button(env.console, BUTTON_ZL, 10, 125);
            uint8_t slot = 0;
            for (; slot < 4; slot++){
                if (pp1[slot] != 0){
                    break;
                }
            }
            if (slot == 4){
                env.log("Ran out of PP in a battle.", COLOR_RED);
                PA_THROW_StringException("Ran out of PP in a battle.");
            }

            while (move_slot1 < slot){
                move_slot1++;
                pbf_press_dpad(env.console, DPAD_DOWN, 10, 50);
            }
            pbf_press_button(env.console, BUTTON_ZL, 10, 125);
            pbf_press_button(env.console, BUTTON_ZL, 10, 375);
            pp1[slot]--;
        }
    }

    env.log("No progress detected after 5 battle menus.", COLOR_RED);
    PA_THROW_StringException("No progress detected after 5 battle menus. Are you out of PP?");
}

void MoneyFarmerRoute210::heal_and_return(ConsoleHandle& console, uint8_t pp0[4], uint8_t pp1[4]){
    console.log("Healing " + STRING_POKEMON + " Celestic Town " + STRING_POKEMON + " Center.");
    pbf_move_left_joystick(console, 125, 0, 6 * TICKS_PER_SECOND, 0);
    pbf_mash_button(console, BUTTON_ZL, 3 * TICKS_PER_SECOND);
    pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);

    console.log("Returning to trainers...");
    pbf_move_left_joystick(console, 128, 255, 6 * TICKS_PER_SECOND, 0);
    pbf_move_left_joystick(console, 255, 128, 60, 0);
    pbf_move_left_joystick(console, 128, 0, 200, 0);
    pbf_move_left_joystick(console, 255, 128, 750, 0);

    pbf_press_button(console, BUTTON_R, 10, 150);
    pbf_mash_button(console, BUTTON_ZL, 6 * TICKS_PER_SECOND);

    pbf_move_left_joystick(console, 128, 255, 30, 0);
    pbf_move_left_joystick(console,   0, 128, 30, 0);
    pbf_move_left_joystick(console, 128, 255, 80, 0);
    pbf_move_left_joystick(console, 255, 128, 110, 0);
    pbf_move_left_joystick(console, 128, 255, 125, 0);
    pbf_move_left_joystick(console, 255, 128, 105, 0);
    pbf_move_left_joystick(console, 128,   0, 375, 0);
    pbf_move_left_joystick(console, 255, 128, 300, 0);
    pbf_move_left_joystick(console, 128, 255, 375, 0);

    pbf_press_dpad(console, DPAD_RIGHT, 375, 0);
    pbf_press_dpad(console, DPAD_LEFT, 375, 0);
    pbf_press_dpad(console, DPAD_DOWN, 125, 0);

    pp0[0] = MON0_MOVE1_PP;
    pp0[1] = MON0_MOVE2_PP;
    pp0[2] = MON0_MOVE3_PP;
    pp0[3] = MON0_MOVE4_PP;
    pp1[0] = MON1_MOVE1_PP;
    pp1[1] = MON1_MOVE2_PP;
    pp1[2] = MON1_MOVE3_PP;
    pp1[3] = MON1_MOVE4_PP;
}
void MoneyFarmerRoute210::flyback_heal_and_return(ConsoleHandle& console, uint8_t pp0[4], uint8_t pp1[4]){
    console.log("Flying back to Hearthome City to heal.");
    pbf_press_button(console, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(console, BUTTON_PLUS, 10, 240);
    pbf_press_dpad(console, DPAD_LEFT, 10, 60);
    pbf_press_dpad(console, DPAD_LEFT, 10, 60);
    pbf_mash_button(console, BUTTON_ZL, 12 * TICKS_PER_SECOND);
    heal_and_return(console, pp0, pp1);
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





void MoneyFarmerRoute210::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

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
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    heal_and_return(env.console, pp0, pp1);
    bool need_to_charge = false;

    while (true){
        env.update_stats();

        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        if (need_to_charge){
            pbf_move_left_joystick(env.console, 255, 128, 140, 0);
            pbf_press_dpad(env.console, DPAD_UP, 85, 0);
            for (size_t c = 0; c < 7; c++){
                pbf_move_left_joystick(env.console, 0, 128, 140, 0);
                pbf_move_left_joystick(env.console, 255, 128, 140, 0);
            }
            pbf_press_dpad(env.console, DPAD_DOWN, 75, 0);
        }
        pbf_press_dpad(env.console, DPAD_LEFT, 200, 0);

        env.console.botbase().wait_for_all_requests();
        stats.m_searches++;

        std::vector<ImagePixelBox> bubbles;
        {
            VSSeekerReactionTracker tracker(env.console, {0.20, 0.20, 0.60, 0.60});
            run_until(
                env, env.console,
                [=](const BotBaseContext& context){
                    SHORTCUT.run(context, TICKS_PER_SECOND);

                },
                { &tracker }
            );
            need_to_charge = true;
            pbf_mash_button(env.console, BUTTON_B, 250);

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

        this->battle(env, pp0, pp1);
        if (!has_pp(pp0, pp1)){
            flyback_heal_and_return(env.console, pp0, pp1);
            need_to_charge = false;
            continue;
        }
    }
}






}
}
}










