/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattleMoveSelectionDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_IngoBattleGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"

#include <QImage>

#include <chrono>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

MoveStyleOption::MoveStyleOption(const char* label) : EnumDropdownOption(
    label,
    {
        "No style",
        "Agile",
        "Strong"
    },
    0
){}

IngoBattleGrinder_Descriptor::IngoBattleGrinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:IngoBattleGrinder",
        STRING_POKEMON + " LA", "Ingo Battle Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/IngoBattleGrinder.md",
        "Attend Ingo's battles to grind exp and move related pokedex research tasks.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


IngoBattleGrinder::IngoBattleGrinder(const IngoBattleGrinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , MOVE1_STYLE_OPTION("<b>Move 1 style:</b>")
    , MOVE2_STYLE_OPTION("<b>Move 2 style:</b>")
    , MOVE3_STYLE_OPTION("<b>Move 3 style:</b>")
    , MOVE4_STYLE_OPTION("<b>Move 4 style:</b>")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MOVE1_STYLE_OPTION);
    PA_ADD_OPTION(MOVE2_STYLE_OPTION);
    PA_ADD_OPTION(MOVE3_STYLE_OPTION);
    PA_ADD_OPTION(MOVE4_STYLE_OPTION);
    
    PA_ADD_OPTION(NOTIFICATIONS);
}



class IngoBattleGrinder::Stats : public StatsTracker{
public:
    Stats()
        : battles(m_stats["Battles"])
        , move_attempts(m_stats["Move Attempts"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Move Attempts");
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> IngoBattleGrinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

void IngoBattleGrinder::use_move(const BotBaseContext &context, int cur_move, bool intended_pokemon_fainted){
    if (intended_pokemon_fainted){
        pbf_press_button(context, BUTTON_A, 10, 125);
        pbf_wait(context, 1 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
        return;
    }

    size_t style = 0;
    switch(cur_move){
        case 0: style = MOVE1_STYLE_OPTION; break;
        case 1: style = MOVE2_STYLE_OPTION; break;
        case 2: style = MOVE3_STYLE_OPTION; break;
        case 3: style = MOVE4_STYLE_OPTION; break;
    }

    // Select move styles
    if (style == 1){
        // Agile style
        pbf_press_button(context, BUTTON_L, 10, 125);
    } else if (style == 2){
        // Strong style
        pbf_press_button(context, BUTTON_R, 10, 125);
    }

    // Choose the move
    pbf_press_button(context, BUTTON_A, 10, 125);
    pbf_wait(context, 1 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
}

bool IngoBattleGrinder::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    stats.battles++;

    env.console.log("Starting battle...");

    // Talk to Ingo to select opponent:
    pbf_mash_button(env.console, BUTTON_A, 10 * TICKS_PER_SECOND);
    pbf_wait(env.console, 5 * TICKS_PER_SECOND);
    pbf_press_button(env.console, BUTTON_B, 10, 115);
    env.console.context().wait_for_all_requests();

    // Which move (0, 1, 2 or 3) to use in next turn.
    int cur_move = 0;
    bool intended_pokemon_fainted = false;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        DialogueEllipseDetector dialogue_ellipse_detector(env.console, env.console, std::chrono::milliseconds(110), stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        int ret = wait_until(
            env, env.console, std::chrono::minutes(2),
            {&battle_menu_detector, &dialogue_ellipse_detector, &pokemon_switch_detector}
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
            return true;

            // throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);
            // Choose move!

            stats.move_attempts++;

            // Press A to select moves
            pbf_press_button(env.console, BUTTON_A, 10, 125);

            use_move(env.console, cur_move, intended_pokemon_fainted);

            // Check if the move cannot be used due to no PP.
            // In this case, we will still be on the move selection screen:
            BattleMoveSelectionDetector move_selection_detector(env.console, env.console, stop_on_detected);
            QImage screen = env.console.video().snapshot();
            if (move_selection_detector.process_frame(screen, std::chrono::system_clock::now())){
                // We are still on the move selection screen. No PP
                if (cur_move == 3){
                    // Pokemon has zero PP on all moves.
                    env.console.log("No PP on all moves. Abort program.", COLOR_RED);
                    throw OperationFailedException(env.console, "No PP on all moves.");
                }
                env.console.log("No PP. Use next move.", COLOR_RED);
                // Go to the next move.
                pbf_press_dpad(env.console, DPAD_DOWN, 20, 100);
                // env.console.context().wait_for_all_requests();
                cur_move++;

                use_move(env.console, cur_move, intended_pokemon_fainted);
            }

            env.update_stats();
        }
        else if (ret == 1){
            env.console.log("End of battle.");

            ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(110), stop_on_detected);
            run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    pbf_mash_button(context, BUTTON_B, 20 * TICKS_PER_SECOND);
                },
                { &arc_phone_detector }
            );
            break; // leave the battle loop
        }
        else{ // ret is 2
            env.console.log("Pokemon fainted.", COLOR_RED);
            intended_pokemon_fainted = true;
            cur_move = 0;
            // Fall back to using the party lead. This pokemon should be very strong.
            // We assume only using its first move with no style to finish the battle.
            pbf_press_button(env.console, BUTTON_A, 20, 100);
            pbf_press_button(env.console, BUTTON_A, 20, 100);
            env.console.context().wait_for_all_requests();
        }
    }

    return false;
}



void IngoBattleGrinder::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);

    // Put a save here so that when the program reloads from error it won't break.
    // save_game_from_overworld(env, env.console);
    // env.console.context().wait_for_all_requests();

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            if (run_iteration(env)){
                break;
            }
        }catch (OperationFailedException&){
            stats.errors++;
            break;
            // pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            // reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }catch (OperationCancelledException&){
            break;
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
