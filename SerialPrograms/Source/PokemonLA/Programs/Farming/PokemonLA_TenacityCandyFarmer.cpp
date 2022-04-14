/*  Tenacity Candy Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <chrono>
#include <iostream>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_TenacityCandyFarmer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TenacityCandyFarmer_Descriptor::TenacityCandyFarmer_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:TenacityCandyFarmer",
        STRING_POKEMON + " LA", "Tenacity Candy Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/TenacityCandyFarmer.md",
        "Attend Ingo's Path of Tenacity battles leading with a stats fully upgraded, max level, Modest nature Arceus with Legend Plate applied to grind exp, exp candies XL and evolution items.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TenacityCandyFarmer::TenacityCandyFarmer(const TenacityCandyFarmer_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , FOURTH_MOVE_ON_MAMOSWINE(
        "<b>Fourth Move on Mamoswine:</b><br>Use fourth move (Flamethrower) on Mamoswine to grind its research task.",
        false
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(FOURTH_MOVE_ON_MAMOSWINE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class TenacityCandyFarmer::Stats : public StatsTracker{
public:
    Stats()
        : battles(m_stats["Battles"])
        , faint_switches(m_stats["Faint Switches"])
        , fourth_moves(m_stats["Fourth Moves"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Faint Switches", true);
        m_display_order.emplace_back("Fourth Moves", true);
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& faint_switches;
    std::atomic<uint64_t>& fourth_moves;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> TenacityCandyFarmer::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

bool TenacityCandyFarmer::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    env.console.log("Starting battle...");

    // Talk to Ingo to start conversation and select Path of Tenacity:
    // Press A to start talking
    pbf_press_button(context, BUTTON_A, 20, 100);
    // Press A to show battle type selection menu box
    pbf_press_button(context, BUTTON_A, 20, 50);
    // Move down the menu box to select Path of Tenacity
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    // Press A to select Path of Tenacity
    pbf_press_button(context, BUTTON_A, 20, 200);
    // Press A to show the opponenet team selection menu box
    pbf_press_button(context, BUTTON_A, 20, 50);
    // Move down the menu box to select Pearl Clan
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    // Select Pearl Plan to start Path of Tenacity
    pbf_mash_button(context, BUTTON_A, 200);
    context.wait_for_all_requests();

    // First opponent, Lian:
    // - Goodra: to be KOed or largely weakened by Judgement
    // - Mamoswine: to be KOed by Energy Ball
    // - Wiscash: to be KOed by Energy Ball
    // Note: Wiscash knows Mud Bomb to decrease accuracy, so the battle may take longer than three turns.
    // Second opponent, Gaeric:
    // - Glalie: to be KOed by Judgement
    // - Avalugg: to be KOed by Flash Cannon
    // - Froslass: to be KOed by Flash Cannon
    // Note: Froslass knows Thunderbolt to inflict paralysis, so the battle may take longer than three turns.
    // Third opponent, Irida:
    // - Glaceon: to be KOed by Judgement
    // - Espeon: to be KOed by Judgement
    // - Flareon: to be KOed by Judgement

    // Assume player's Arceus has move:
    // 0: Judgement
    // 1: Energy Ball
    // 2: Flash Cannon
    const size_t target_battle_moves[3][3] = {
        {0, 1, 1},
        {0, 2, 2},
        {0, 0, 0},
    };

    // Which move (0, 1, 2 or 3) the game currently selects
    size_t cur_move = 0;
    // The battle-order index of the current pokemon on the battle field.
    size_t cur_pokemon = 0;
    // How many turns have passed for the current pokemon in this battle.
    int num_turns = 0;
    // Used to skip fainted pokemon in the party when switching a pokemon
    // This is the party-order index of the pokemon to switch to.
    // The index is the index in the pokemon party list.
    size_t next_pokemon_to_switch_to = 0;

    // which battle is currently on (vs Lian, Gaeric or Irida)
    size_t cur_battle = 0;
    // Whether we are in a state to clear multiple dialogue boxes.
    bool clearing_dialogues = true;

    auto clear_dialogue_box = [&](){
        if (clearing_dialogues == false){
            // We just ended one battle:
            cur_battle++;
            cur_move = 0;
            cur_pokemon = 0;
            num_turns = 0;
        }
        clearing_dialogues = true;
        pbf_press_button(context, BUTTON_B, 20, 100);
        context.wait_for_all_requests();

        if (cur_battle >= 3){
            env.log("All three battles finished. Mashing B to clear dialogues.");
            // All three battles are now finished, wait for ArcPhoneDetector
            const bool stop_on_detected = true;
            ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
            int ret = run_until(
                env.console, context, [](BotBaseContext& context){
                    pbf_mash_button(context, BUTTON_B, 20 * TICKS_PER_SECOND);
                },
                {{arc_phone_detector}}
            );
            if (ret < 0){
                throw OperationFailedException(env.console, "Failed to find Arc phone after 20 seconds when the last battle ends.");
            }
            env.log("Found Arc Phone. End of one path.");

            return true;
        }
        return false;
    };

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        // dialogue ellipse appears on a semi-transparent dialog box if you win the fight.
        DialogueEllipseDetector dialogue_ellipse_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        // normal dialogue appears if you lose the fight.
        NormalDialogDetector normal_dialogue_detector(env.console, env.console, stop_on_detected);
        DialogSurpriseDetector surprise_dialogue_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        int ret = wait_until(
            env.console, context, std::chrono::minutes(2),
            {
                {battle_menu_detector},
                {dialogue_ellipse_detector},
                {normal_dialogue_detector},
                {surprise_dialogue_detector},
                {pokemon_switch_detector},
                {arc_phone_detector},
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
//            return true;
            throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);
            clearing_dialogues = false;

            // Press A to select moves
            pbf_press_button(context, BUTTON_A, 10, 125);
            context.wait_for_all_requests();

            // Choose move to use!
            if (cur_pokemon == 0){ // Arceus is still alive
                size_t target_move = target_battle_moves[cur_battle][std::min(num_turns, 2)];

                if (FOURTH_MOVE_ON_MAMOSWINE && cur_battle == 0 && num_turns == 1){
                    target_move = 3;
                    stats.fourth_moves++;
                    env.update_stats();
                }

                if (cur_move < target_move){
                    // Move move selection down
                    for(size_t i = 0; i < target_move - cur_move; i++){
                        pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                    }
                } else if (cur_move > target_move){
                    // Move move selection up
                    for(size_t i = 0; i < cur_move - target_move; i++){
                        pbf_press_dpad(context, DPAD_UP, 20, 100);
                    }
                }
                cur_move = target_move;
            }

            const MoveStyle no_style = MoveStyle::NoStyle;
            const bool check_move_success = true;
            while (use_move(env.console, context, cur_pokemon, cur_move, no_style, check_move_success) == false){
                // We are still on the move selection screen. No PP.
                // Go to the next move.
                pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                // env.console.context().wait_for_all_requests();
                cur_move = (cur_move + 1) % 4;
                env.console.log("No PP. Use next move, " + QString::number(cur_move), COLOR_RED);
            }

            num_turns++;
        }
        else if (ret == 1){
            env.console.log("Transparent dialogue box.");
            if (clear_dialogue_box()){
                break;
            }
        } else if(ret == 2){
            env.console.log("Normal dialogue box.");
            if (clear_dialogue_box()){
                break;
            }
        } else if(ret == 3){
            env.console.log("Surprise dialogue box.");
            if (clear_dialogue_box()){
                break;
            }
        }
        else if (ret == 4){
            env.console.log("Pokemon fainted.", COLOR_RED);

            clearing_dialogues = false;
            stats.faint_switches++;
            env.update_stats();

            cur_move = 0;
            next_pokemon_to_switch_to++;
            next_pokemon_to_switch_to = switch_pokemon(env.console, context, next_pokemon_to_switch_to);
            cur_pokemon++;
        }
        else{ // ret is 5
            env.console.log("Battle finished.");
            break;
        }
    }

    stats.battles++;
    env.update_stats();

    return false;
}



void TenacityCandyFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    // {
    //     // QImage image("./scripts/LA_switch_pokemon_Kuro.png");
    //     QImage image("./PLA_test_data/ingoBattle/broken_dialogue_detector.png");
    //     const bool stop_on_detected = true;
    //     NormalDialogDetector detector(env.console, env.console, stop_on_detected);
    //     bool detected = detector.process_frame(image, current_time());
    //     std::cout << "detector " << detected << std::endl;
    //     return;
    // }

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            if (run_iteration(env, context)){
                break;
            }
        }catch (OperationFailedException&){
            stats.errors++;
            throw;
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}





}
}
}
