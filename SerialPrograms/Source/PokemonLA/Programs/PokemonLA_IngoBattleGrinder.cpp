/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <chrono>
#include <QImage>
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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



const char* INGO_OPPONENT_STRINGS[] = {
    "Wenton",
    "Bren",
    "Zisu",
    "Akari/Rei",
    "Kamado",
    "Beni",
    "Ingo",
    "Ingo - but tougher",
    "Mai",
    "Sabi",
    "Ress",
    "Ingo - but using alphas",
};
const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V10[] = {
    {0, 0},     //  Wenton
    {0, 1},     //  Bren
    {0, 2},     //  Zisu
    {0, 3},     //  Akari/Rei
    {1, 0},     //  Kamado
    {1, 1},     //  Beni
    {1, 2},     //  Ingo
    {-1, -1},   //  Ingo - but tougher
    {-1, -1},   //  Mai
    {-1, -1},   //  Sabi
    {-1, -1},   //  Ress
    {-1, -1},   //  Ingo - but using alphas
};
const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V12[] = {
    {0, 0},     //  Wenton
    {0, 1},     //  Bren
    {0, 2},     //  Zisu
    {0, 3},     //  Akari/Rei
    {0, 4},     //  Kamado
    {1, 0},     //  Beni
    {1, 1},     //  Ingo
    {1, 2},     //  Ingo - but tougher
    {1, 3},     //  Mai
    {1, 4},     //  Sabi
    {2, 0},     //  Ress
    {2, 1},     //  Ingo - but using alphas
};



MoveStyleOption::MoveStyleOption(const char* label) : EnumDropdownOption(
    label,
    {
        "No style",
        "Agile",
        "Strong"
    },
    0
){}


PokemonBattleDecisionOption::PokemonBattleDecisionOption(QString description): 
    GroupOption(std::move(description))
    , MOVE_1("<b>Move 1 style:</b>")
    , MOVE_2("<b>Move 2 style:</b>")
    , MOVE_3("<b>Move 3 style:</b>")
    , MOVE_4("<b>Move 4 style:</b>")
    , SWITCH("<b>Switch this " + STRING_POKEMON + " in battle:</b>", false)
    , NUM_TURNS_TO_SWITCH("<b>Num turns to switch:</b>", 1)
{
    PA_ADD_OPTION(MOVE_1);
    PA_ADD_OPTION(MOVE_2);
    PA_ADD_OPTION(MOVE_3);
    PA_ADD_OPTION(MOVE_4);
    PA_ADD_OPTION(SWITCH);
    PA_ADD_OPTION(NUM_TURNS_TO_SWITCH);
}


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
    , OPPONENT(
        "<b>Opponent:</b>",
        std::vector<QString>(INGO_OPPONENT_STRINGS, INGO_OPPONENT_STRINGS + (size_t)IngoOpponents::END_LIST),
        0
    )
    , POKEMON_1("First " + STRING_POKEMON)
    , POKEMON_2("Second " + STRING_POKEMON)
    , POKEMON_3("Third " + STRING_POKEMON)
    , POKEMON_4("Fourth " + STRING_POKEMON)
    , POKEMON_5("Remaining " + STRING_POKEMON)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(OPPONENT);
    PA_ADD_OPTION(POKEMON_1);
    PA_ADD_OPTION(POKEMON_2);
    PA_ADD_OPTION(POKEMON_3);
    PA_ADD_OPTION(POKEMON_4);
    PA_ADD_OPTION(POKEMON_5);
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

bool IngoBattleGrinder::start_dialog(SingleSwitchProgramEnvironment& env){
    {
        ButtonDetector button0(env.console, env.console, ButtonType::ButtonA, {0.50, 0.450, 0.40, 0.042}, std::chrono::milliseconds(100), true);
        ButtonDetector button1(env.console, env.console, ButtonType::ButtonA, {0.50, 0.492, 0.40, 0.042}, std::chrono::milliseconds(100), true);
        int ret = run_until(
            env, env.console,
            [&](const BotBaseContext& context){
                for (size_t c = 0; c < 10; c++){
                    pbf_press_button(context, BUTTON_A, 20, 150);
                }
            },
            { &button0, &button1 }
        );
        switch (ret){
        case 0:
            return true;
        case 1:
            break;
        default:
            throw OperationFailedException(env.console, "Unable to detect options after 10 A presses.");
        }
    }

    pbf_press_button(env.console, BUTTON_A, 20, 150);
    env.console.botbase().wait_for_all_requests();

    ButtonDetector button2(env.console, env.console, ButtonType::ButtonA, {0.50, 0.350, 0.40, 0.400}, std::chrono::milliseconds(100), true);
    int ret = run_until(
        env, env.console,
        [&](const BotBaseContext& context){
            for (size_t c = 0; c < 5; c++){
                pbf_press_button(context, BUTTON_A, 20, 150);
            }
        },
        { &button2 }
    );
    switch (ret){
    case 0:
        return false;
    default:
        throw OperationFailedException(env.console, "Unable to opponent list options after 5 A presses.");
    }
}

void IngoBattleGrinder::use_move(const BotBaseContext &context, int cur_pokemon, int cur_move){
    const PokemonBattleDecisionOption* pokemon = get_pokemon(cur_pokemon);

    size_t style = 0;
    switch(cur_move){
        case 0: style = pokemon->MOVE_1; break;
        case 1: style = pokemon->MOVE_2; break;
        case 2: style = pokemon->MOVE_3; break;
        case 3: style = pokemon->MOVE_4; break;
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

void IngoBattleGrinder::switch_pokemon(SingleSwitchProgramEnvironment& env, int& next_pokemon_in_party_order){
    // Move fast leading fainted pokemon
    for(int i = 0; i < next_pokemon_in_party_order; i++){
        pbf_press_dpad(env.console, DPAD_DOWN, 20, 80);
    }

    while(true){
        // Fall back to using the party lead. This pokemon should be very strong.
        // We assume only using its first move with no style to finish the battle.
        pbf_press_button(env.console, BUTTON_A, 20, 100);
        pbf_press_button(env.console, BUTTON_A, 20, 150);
        env.console.context().wait_for_all_requests();

        next_pokemon_in_party_order++;
    
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

const PokemonBattleDecisionOption* IngoBattleGrinder::get_pokemon(int cur_pokemon) const {
    switch (cur_pokemon){
        case 0: return &POKEMON_1;
        case 1: return &POKEMON_2;
        case 2: return &POKEMON_3;
        case 3: return &POKEMON_4;
        default: return &POKEMON_5;
    }
}

bool IngoBattleGrinder::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    stats.battles++;

    env.console.log("Starting battle...");

    // Talk to Ingo to start conversation and select regular battles:
    bool version_10 = start_dialog(env);
    IngoOpponentMenuLocation menu_location = version_10
        ? INGO_OPPONENT_MENU_LOCATIONS_V10[OPPONENT]
        : INGO_OPPONENT_MENU_LOCATIONS_V10[OPPONENT];

    // Choose which opponent
    if (menu_location.page < 0){
        throw UserSetupError(env.console, "Opponent doesn't exist in this version of the game.");
    }

    //  Move to page.
    for (int8_t c = 0; c < menu_location.page; c++){
        pbf_press_dpad(env.console, DPAD_UP, 10, 60);
        pbf_press_dpad(env.console, DPAD_UP, 10, 60);
        pbf_press_button(env.console, BUTTON_A, 10, 100);
    }

    //  Move to slot.
    for (int8_t c = 0; c < menu_location.index; c++){
        pbf_press_dpad(env.console, DPAD_DOWN, 10, 60);
    }

    // Press the button to select the opponent
    pbf_press_button(env.console, BUTTON_A, 10, 115);
    pbf_wait(env.console, 1 * TICKS_PER_SECOND);
    env.console.context().wait_for_all_requests();

    // Which move (0, 1, 2 or 3) to use in next turn.
    int cur_move = 0;
    // Which pokemon in the party is not fainted
    int cur_pokemon = 0;
    // How many turns have passed in this battle:
    int num_turns = 0;
    // Used to skip fainted pokemon in the party when switching a pokemon
    int next_pokemon_in_party_order = 0;

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

            // User may want to switch the pokemon after some turns, to get more exp, or prevent if from
            // fainting.
            const PokemonBattleDecisionOption* pokemon = get_pokemon(cur_pokemon);
            if (pokemon->SWITCH && pokemon->NUM_TURNS_TO_SWITCH == num_turns){
                env.console.log("Switch pokemon");

                // Go to the switching pokemon screen:
                pbf_press_dpad(env.console, DPAD_DOWN, 20, 100);

                cur_move = 0;
                num_turns = 0;
                switch_pokemon(env, next_pokemon_in_party_order);
                cur_pokemon++;
            }
            else{
                // Choose move to use!
                if (cur_pokemon == 0){
                    // We collect the stat of move attempts of the first pokemon.
                    stats.move_attempts++;
                }

                // Press A to select moves
                pbf_press_button(env.console, BUTTON_A, 10, 125);

                use_move(env.console, cur_pokemon, cur_move);

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

                    use_move(env.console, cur_pokemon, cur_move);
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

            cur_move = 0;
            num_turns = 0;
            switch_pokemon(env, next_pokemon_in_party_order);
            cur_pokemon++;
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
