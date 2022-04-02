/*  Ingo Battle Grinder
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
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_MagikarpMoveGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


MagikarpMoveGrinder_Descriptor::MagikarpMoveGrinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:MagikarpMoveGrinder",
        STRING_POKEMON + " LA", "Magikarp Move Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/MagikarpMoveGrinder.md",
        "grind status moves with any style against a Magikarp to finish " + STRING_POKEDEX + " research tasks.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MagikarpMoveGrinder::MagikarpMoveGrinder(const MagikarpMoveGrinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(POKEMON_ACTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class MagikarpMoveGrinder::Stats : public StatsTracker{
public:
    Stats()
        : magikarp(m_stats["Magikarp"])
        , move_attempts(m_stats["Move Attempts"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Magikarp");
        m_display_order.emplace_back("Move Attempts");
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& magikarp;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> MagikarpMoveGrinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void MagikarpMoveGrinder::switch_pokemon(BotBaseContext& context, ConsoleHandle& console, size_t& next_pokemon_in_party_order){
    // Move past leading fainted pokemon
    for(size_t i = 0; i < next_pokemon_in_party_order; i++){
        pbf_press_dpad(context, DPAD_DOWN, 20, 80);
    }

    while(true){
        // Choose the next pokemon to battle.
        pbf_press_button(context, BUTTON_A, 20, 100);
        pbf_press_button(context, BUTTON_A, 20, 150);
        context.wait_for_all_requests();

        // Check whether we can send this pokemon to battle:
        const bool stop_on_detected = true;
        BattlePokemonSwitchDetector switch_detector(console, console, stop_on_detected);
        QImage screen = console.video().snapshot();
        if (switch_detector.process_frame(screen, std::chrono::system_clock::now()) == false){
            // No longer at the switching pokemon screen
            break;
        }

        // We are still in the switching pokemon screen. So the current selected pokemon is fainted
        // and therefore cannot be used. Try the next pokemon:
        next_pokemon_in_party_order++;
        if (next_pokemon_in_party_order >= POKEMON_ACTIONS.num_pokemon()){
            throw OperationFailedException(console, "Cannot send any more Pokemon to battle.");
        }

        // Fist hit B to clear the "cannot send pokemon" dialogue
        pbf_press_button(context, BUTTON_B, 20, 100);
        // Move to the next pokemon
        pbf_press_dpad(context, DPAD_DOWN, 20, 80);
    }
}

bool MagikarpMoveGrinder::battle_magikarp(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    // The location of the first move slot when choosing which move to use during battle.
    // This box will be used to check whether the content in those boxes are changed or not
    // after selecting one move to use. In this way we can detect whether the move is out of PP.
    const ImageFloatBox move_slot_box = {0.6600, 0.6220, 0.2500, 0.0320};

    env.console.log("Grinding on Magikarp...");

    // Which pokemon in the party is not fainted
    size_t cur_pokemon = 0;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        int ret = wait_until(
            env, context, env.console, std::chrono::minutes(2),
            {
                &battle_menu_detector,
                &pokemon_switch_detector,
                &arc_phone_detector
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
            throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);

            // Press A to select moves
            pbf_press_button(context, BUTTON_A, 10, 125);
            context.wait_for_all_requests();

            QImage screen = env.console.video().snapshot();
            ImageMatchDetector move_slot_detector(std::move(screen), move_slot_box, 10.0);
            
            const MoveStyle style = POKEMON_ACTIONS.get_style(cur_pokemon);

            // Select move styles
            if (style == MoveStyle::Agile){
                // Agile style
                pbf_press_button(context, BUTTON_L, 10, 125);
            } else if (style == MoveStyle::Strong){
                // Strong style
                pbf_press_button(context, BUTTON_R, 10, 125);
            }

            env.log("Using pokemon " + QString::number(cur_pokemon) + "/" + QString::number(POKEMON_ACTIONS.num_pokemon()) + 
                " style " + MoveStyle_NAMES[(int)style]);

            // Choose the move
            pbf_press_button(context, BUTTON_A, 10, 125);
            pbf_wait(context, 1 * TICKS_PER_SECOND);
            context.wait_for_all_requests();

            // Check if the move cannot be used due to no PP:
            screen = env.console.video().snapshot();
            if (move_slot_detector.detect(screen)){
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
                
                switch_pokemon(context, env.console, cur_pokemon);

#ifdef DEBUG_NO_PP
                static int count = 0;
                screen.save("./no_pp." + QString::number(count++) + ".png");
#endif
            }
            else{
                stats.move_attempts++;
                env.update_stats();
            }
        }
        else if (ret == 1){
            env.console.log("Pokemon fainted.");
            cur_pokemon++;
            if (cur_pokemon >= POKEMON_ACTIONS.num_pokemon()){
                env.console.log("All pokemon grinded. Stop program.");
                break;
            }
            switch_pokemon(context, env.console, cur_pokemon);
        }
        else{ // ret is 2
            env.console.log("Battle finished.");
            break;
        }
    }

    stats.magikarp++;
    env.update_stats();

    return false;
}



void MagikarpMoveGrinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    if (POKEMON_ACTIONS.num_pokemon() == 0){
        throw OperationFailedException(env.console, "No Pokemon sepecified to grind.");
    }

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    env.update_stats();
    send_program_status_notification(
        env.logger(), NOTIFICATION_STATUS,
        env.program_info(),
        "",
        stats.to_str()
    );
    try{
        battle_magikarp(env, context);
    }catch (OperationFailedException&){
        stats.errors++;
        throw;
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
