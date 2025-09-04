/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <iostream>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA_MagikarpMoveGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


MagikarpMoveGrinder_Descriptor::MagikarpMoveGrinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:MagikarpMoveGrinder",
        STRING_POKEMON + " LA", "Magikarp Move Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/MagikarpMoveGrinder.md",
        "grind status moves with any style against a Magikarp to finish " + STRING_POKEDEX + " research tasks.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class MagikarpMoveGrinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : magikarp(m_stats["Magikarp"])
        , move_attempts(m_stats["Move Attempts"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Magikarp");
        m_display_order.emplace_back("Move Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& magikarp;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> MagikarpMoveGrinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MagikarpMoveGrinder::MagikarpMoveGrinder()
    : SPECIAL_CASE_MIMIC(
        "<b>Special Case, Mimic:</b><br>Grind Mimic move usages by switching between first two " + STRING_POKEMON + ". Set the first move as Mimic on the first two " + STRING_POKEMON + ".<br>"
        "After switching, the retreated " + STRING_POKEMON + " will forget the move learned by Mimic, allowing efficient Mimic grinding.<br>"
        "Choosing this will ignore the content in " + STRING_POKEMON + " Action Table.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(POKEMON_ACTIONS);
    PA_ADD_OPTION(SPECIAL_CASE_MIMIC);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void MagikarpMoveGrinder::grind_mimic(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MagikarpMoveGrinder_Descriptor::Stats& stats = env.current_stats<MagikarpMoveGrinder_Descriptor::Stats>();
    env.log("Special case: grinding Mimic...");

    // Which pokemon in the party is not fainted
    size_t cur_pokemon = 0;
    // Whether to switch the pokemon next turn
    bool to_switch_pokemon = false;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        int ret = wait_until(
            env.console, context, std::chrono::minutes(2),
            {
                {battle_menu_detector},
                {pokemon_switch_detector},
                {arc_phone_detector},
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
//            auto snapshot = env.console.video().snapshot();
//            dump_image(env.logger(), env.program_info(), "BattleMenuNotFound", snapshot);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to find battle menu after 2 minutes.",
                env.console
            );
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);

            if (to_switch_pokemon){
                cur_pokemon = (cur_pokemon+1) % 2;
                env.console.log("Switch Pokemon after Mimic used.", COLOR_RED);;

                // Go to the switching pokemon screen:
                pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                
                cur_pokemon = switch_pokemon(env.console, context, cur_pokemon);

                to_switch_pokemon = false;
            }else{
                // Press A to select moves
                pbf_press_button(context, BUTTON_A, 10, 125);
                context.wait_for_all_requests();
                
                const MoveStyle style = MoveStyle::NoStyle;
                const bool check_move_success = true;
                if (use_move(env.console, context, cur_pokemon, 0, style, check_move_success) == false){
                    // Finish grinding.
                    env.log("No PP. Finish grinding.");
                    return;
                }else{
                    stats.move_attempts++;
                    env.update_stats();

                    to_switch_pokemon = true;
                }
            }
        }else if (ret == 1){
            env.log("Your pokemon fainted. Can only happen if Magikarp Struggled and defeated your pokemon.");
            return;
        }else{ // ret is 2
            env.log("Battle finished.");
            return;
        }
    }
}

void MagikarpMoveGrinder::battle_magikarp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MagikarpMoveGrinder_Descriptor::Stats& stats = env.current_stats<MagikarpMoveGrinder_Descriptor::Stats>();

    // Which pokemon in the party is not fainted
    size_t cur_pokemon = 0;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        int ret = wait_until(
            env.console, context, std::chrono::minutes(2),
            {
                {battle_menu_detector},
                {pokemon_switch_detector},
                {arc_phone_detector},
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
//            auto snapshot = env.console.video().snapshot();
//            dump_image(env.logger(), env.program_info(), "BattleMenuNotFound", snapshot);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to find battle menu after 2 minutes.",
                env.console
            );
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);

            // Press A to select moves
            pbf_press_button(context, BUTTON_A, 10, 125);
            context.wait_for_all_requests();
            
            const MoveStyle style = POKEMON_ACTIONS.get_style(cur_pokemon);

            const bool check_move_success = true;
            if (use_move(env.console, context, cur_pokemon, 0, style, check_move_success) == false){
                // We are still on the move selection screen. No PP.
                cur_pokemon++;
                if (cur_pokemon >= POKEMON_ACTIONS.num_pokemon()){
                    env.console.log("All pokemon grinded. Stop program.");
                    break;
                }
                env.console.log("No PP. Switch Pokemon.", COLOR_RED);

                // Press B to leave move selection menu
                pbf_press_button(context, BUTTON_B, 10, 125);

                // Go to the switching pokemon screen:
                pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                
                cur_pokemon = switch_pokemon(env.console, context, cur_pokemon, POKEMON_ACTIONS.num_pokemon());
            }else{
                stats.move_attempts++;
                env.update_stats();
            }
        }else if (ret == 1){
            env.console.log("Pokemon fainted.");
            cur_pokemon++;
            if (cur_pokemon >= POKEMON_ACTIONS.num_pokemon()){
                env.console.log("All pokemon grinded. Stop program.");
                break;
            }
            cur_pokemon = switch_pokemon(env.console, context, cur_pokemon, POKEMON_ACTIONS.num_pokemon());
        }else{ // ret is 2
            env.console.log("Battle finished.");
            break;
        }
    }
}



void MagikarpMoveGrinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MagikarpMoveGrinder_Descriptor::Stats& stats = env.current_stats<MagikarpMoveGrinder_Descriptor::Stats>();

    if (POKEMON_ACTIONS.num_pokemon() == 0){
        throw UserSetupError(env.console, "No Pokemon specified to grind.");
    }

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS);
    env.console.log("Grinding on Magikarp...");

    try{
        if (SPECIAL_CASE_MIMIC){
            grind_mimic(env, context);
        }else{
            battle_magikarp(env, context);
        }

        stats.magikarp++;
        env.update_stats();
    }catch (OperationFailedException&){
        stats.errors++;
        throw;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
