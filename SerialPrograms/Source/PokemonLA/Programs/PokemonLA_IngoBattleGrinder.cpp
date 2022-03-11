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
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"

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
    , OPPONENT("<b>Opponent:</b>",
    {
        "Wenton",
        "Bren",
        "Zisu",
        "Akari/Rei",
        "Kamado",
        // ---- new page
        "Beni",
        "Ingo",
        "Ingo - but tougher",
        "Mai",
        "Sabi",
        // ---- new page
        "Ress",
        "Ingo - but using alphas"
    }, 0)
    , MOVE1_STYLE_OPTION("<b>Move 1 style:</b>")
    , MOVE2_STYLE_OPTION("<b>Move 2 style:</b>")
    , MOVE3_STYLE_OPTION("<b>Move 3 style:</b>")
    , MOVE4_STYLE_OPTION("<b>Move 4 style:</b>")
    , SWITCH_FIRST_POKEMON("<b>Switch first " + STRING_POKEMON + " in battle:</b>", false)
    , NUM_TURNS_TO_SWITCH("<b>Num turns to switch first " + STRING_POKEMON + ":</b>", 1)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(OPPONENT);
    PA_ADD_OPTION(MOVE1_STYLE_OPTION);
    PA_ADD_OPTION(MOVE2_STYLE_OPTION);
    PA_ADD_OPTION(MOVE3_STYLE_OPTION);
    PA_ADD_OPTION(MOVE4_STYLE_OPTION);
    PA_ADD_OPTION(SWITCH_FIRST_POKEMON);
    PA_ADD_OPTION(NUM_TURNS_TO_SWITCH);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class IngoBattleGrinder::Stats : public StatsTracker{
public:
    Stats()
        : battles(m_stats["Battles"])
        , turns(m_stats["Turns"])
        , move_attempts(m_stats["Move Attempts"])
        , faint_switches(m_stats["Faint Switches"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Turns");
        m_display_order.emplace_back("Move Attempts");
        m_display_order.emplace_back("Faint Switches", true);
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& turns;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& faint_switches;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> IngoBattleGrinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

void IngoBattleGrinder::use_move(const BotBaseContext &context, int cur_move, bool intended_pokemon_left){
    if (intended_pokemon_left){
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

void IngoBattleGrinder::switch_pokemon(SingleSwitchProgramEnvironment& env, int& next_non_fainted_pokemon){
    // Move fast leading fainted pokemon
    for(int i = 0; i < next_non_fainted_pokemon; i++){
        pbf_press_dpad(env.console, DPAD_DOWN, 20, 80);
    }

    while(true){
        // Fall back to using the party lead. This pokemon should be very strong.
        // We assume only using its first move with no style to finish the battle.
        pbf_press_button(env.console, BUTTON_A, 20, 100);
        pbf_press_button(env.console, BUTTON_A, 20, 150);
        env.console.context().wait_for_all_requests();

        next_non_fainted_pokemon++;
    
        // Check whether we can send this pokemon to battle:
        const bool stop_on_detected = true;
        BattlePokemonSwitchDetector switch_detector(env.console, env.console, stop_on_detected);
        QImage screen = env.console.video().snapshot();
        if (switch_detector.process_frame(screen, std::chrono::system_clock::now()) == false){
            // No longer at the switching pokemon screen
            break;
        }

        // We are still in the switching pokemon screen. So the current selected pokemon is fainted
        // and therefore cannot be used. Try the next pokemon:

        // Fist hit B to clear the "cannot send pokemon" dialogue
        pbf_press_button(env.console, BUTTON_B, 20, 100);
        // Move to the next pokemon
        pbf_press_dpad(env.console, DPAD_DOWN, 20, 80);
    }
}

bool IngoBattleGrinder::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    stats.battles++;

    env.console.log("Starting battle...");

    // Talk to Ingo to start conversation and select regular battles:
    pbf_press_button(env.console, BUTTON_A, 20, 100);
    pbf_press_button(env.console, BUTTON_A, 20, 100);
    pbf_press_button(env.console, BUTTON_A, 20, 150);
    pbf_press_button(env.console, BUTTON_A, 20, 120);
    // Choose which opponent
    if (OPPONENT < 5){
        for(size_t i = 0; i < OPPONENT; i++){
            pbf_press_dpad(env.console, DPAD_DOWN, 10, 60);
        }
    } else{
        // Go to next page
        for(size_t i = 0; i < 2; i++){
            pbf_press_dpad(env.console, DPAD_UP, 10, 60);
        }
        pbf_press_button(env.console, BUTTON_A, 10, 100);

        if (OPPONENT < 10){
            for(size_t i = 5; i < OPPONENT; i++){
                pbf_press_dpad(env.console, DPAD_DOWN, 10, 60);
            }
        }
        else{
            // Go to next page
            for(size_t i = 0; i < 2; i++){
                pbf_press_dpad(env.console, DPAD_UP, 10, 60);
            }
            pbf_press_button(env.console, BUTTON_A, 10, 100);

            for(size_t i = 10; i < OPPONENT; i++){
                pbf_press_dpad(env.console, DPAD_DOWN, 10, 60);
            }
        }
    }

    // Press the button to select the opponent
    pbf_press_button(env.console, BUTTON_A, 10, 115);
    pbf_wait(env.console, 1 * TICKS_PER_SECOND);
    env.console.context().wait_for_all_requests();

    // Which move (0, 1, 2 or 3) to use in next turn.
    int cur_move = 0;
    // Track whether the intended pokemon, the first pokemon sent to battle, has fainted or switched:
    bool intended_pokemon_left = false;
    // Which pokemon in the party is not fainted
    int next_non_fainted_pokemon = 0;
    // How many turns have passed in this battle:
    int num_turns = 0;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        // dialogue ellipse appears on a semi-transparent dialog box if you win the fight.
        DialogueEllipseDetector dialogue_ellipse_detector(env.console, env.console, std::chrono::milliseconds(110), stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        // normal dialogue appears if you los the fight.
        NormalDialogDetector normal_dialogue_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(110), stop_on_detected);
        int ret = wait_until(
            env, env.console, std::chrono::minutes(2),
            {
                &battle_menu_detector,
                &dialogue_ellipse_detector,
                &normal_dialogue_detector,
                &pokemon_switch_detector,
                &arc_phone_detector
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
            return true;
            // throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);
            stats.turns++;

            // User may want to switch the first pokemon after some turns, to get more exp, or prevent if from
            // fainting.
            // If next_non_fainted_pokemon is not 0, then the first pokemon is already fainted, we don't need to
            // switch again.
            if (SWITCH_FIRST_POKEMON && NUM_TURNS_TO_SWITCH == num_turns && next_non_fainted_pokemon == 0){
                env.console.log("Switch pokemon");

                // Go to the switching pokemon screen:
                pbf_press_dpad(env.console, DPAD_DOWN, 20, 100);

                intended_pokemon_left = true;
                cur_move = 0;
                switch_pokemon(env, next_non_fainted_pokemon);
            }
            else{
                // Choose move to use!
                if (!intended_pokemon_left){
                    stats.move_attempts++;
                }

                // Press A to select moves
                pbf_press_button(env.console, BUTTON_A, 10, 125);

                use_move(env.console, cur_move, intended_pokemon_left);

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

                    use_move(env.console, cur_move, intended_pokemon_left);
                }
            }

            env.update_stats();
            num_turns++;
        }
        else if (ret == 1 || ret == 2){
            env.console.log("Dialogue box.");

            pbf_press_button(env.console, BUTTON_B, 20, 100);
            env.console.context().wait_for_all_requests();
        }
        else if (ret == 3){
            env.console.log("Pokemon fainted.", COLOR_RED);
            stats.faint_switches++;
            env.update_stats();

            intended_pokemon_left = true;
            cur_move = 0;
            switch_pokemon(env, next_non_fainted_pokemon);
        }
        else{ // ret is 4
            env.console.log("Battle finished.");
            break;
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
