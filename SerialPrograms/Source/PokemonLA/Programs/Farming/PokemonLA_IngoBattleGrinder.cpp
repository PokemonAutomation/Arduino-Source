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
#include "PokemonLA_IngoBattleGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// #define DEBUG_INGO_BATTLE


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



IngoBattleGrinder_Descriptor::IngoBattleGrinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:IngoBattleGrinder",
        STRING_POKEMON + " LA", "Ingo Battle Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/IngoBattleGrinder.md",
        "Attend Ingo's battles to grind exp and move related " + STRING_POKEDEX + " research tasks.",
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
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(OPPONENT);
    PA_ADD_OPTION(POKEMON_ACTIONS);

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
        throw OperationFailedException(env.console, "Unable to find opponent list options after 5 A presses.");
    }
}

void IngoBattleGrinder::use_move(const BotBaseContext &context, size_t cur_pokemon, size_t cur_move){
    const MoveStyle style = POKEMON_ACTIONS.get_style(cur_pokemon, cur_move);

    // Select move styles
    if (style == MoveStyle::Agile){
        // Agile style
        pbf_press_button(context, BUTTON_L, 10, 125);
    } else if (style == MoveStyle::Strong){
        // Strong style
        pbf_press_button(context, BUTTON_R, 10, 125);
    }
    
    std::cout << "Use pokemon " << cur_pokemon << " move " << cur_move << " style " << 
        MoveStyle_NAMES[(int)style].toStdString() << std::endl;

    // Choose the move
    pbf_press_button(context, BUTTON_A, 10, 125);
    pbf_wait(context, 1 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
}

void IngoBattleGrinder::switch_pokemon(SingleSwitchProgramEnvironment& env, size_t& next_pokemon_in_party_order){
    // Move past leading fainted pokemon
    for(size_t i = 0; i < next_pokemon_in_party_order; i++){
        pbf_press_dpad(env.console, DPAD_DOWN, 20, 80);
    }

    while(true){
        // Choose the next pokemon to battle.
        pbf_press_button(env.console, BUTTON_A, 20, 100);
        pbf_press_button(env.console, BUTTON_A, 20, 150);
        env.console.botbase().wait_for_all_requests();

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

bool IngoBattleGrinder::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    // The location of the move slots when choosing which move to use during battle.
    // These boxes will be used to check whether the content in those boxes are changed or not
    // after selecting one move to use. In this way we can detect whether the move is out of PP.
    const ImageFloatBox move_slot_boxes[4] = {
        {0.6600, 0.6220, 0.2500, 0.0320},
        {0.6395, 0.6875, 0.2500, 0.0320},
        {0.6190, 0.7530, 0.2500, 0.0320},
        {0.5985, 0.8185, 0.2500, 0.0320},
    };

    env.console.log("Starting battle...");

    // Talk to Ingo to start conversation and select regular battles:
    // The dialogues are different between version 10 (the vanilla version) and later versions.
    bool version_10 = start_dialog(env);

    IngoOpponentMenuLocation menu_location = version_10
        ? INGO_OPPONENT_MENU_LOCATIONS_V10[OPPONENT]
        : INGO_OPPONENT_MENU_LOCATIONS_V12[OPPONENT];

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
    size_t cur_move = 0;
    // Which pokemon in the party is not fainted
    size_t cur_pokemon = 0;
    // How many turns have passed in this battle:
    size_t num_turns = 0;
    // Used to skip fainted pokemon in the party when switching a pokemon
    size_t next_pokemon_in_party_order = 0;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        // dialogue ellipse appears on a semi-transparent dialog box if you win the fight.
        DialogueEllipseDetector dialogue_ellipse_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        // normal dialogue appears if you lose the fight.
        NormalDialogDetector normal_dialogue_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
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
//            return true;
            throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);
            stats.turns++;

            // User may want to switch the pokemon after some turns, to get more exp, or prevent if from
            // fainting.
            if (POKEMON_ACTIONS.switch_pokemon(cur_pokemon, num_turns)){
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
                env.console.botbase().wait_for_all_requests();

                const auto& move_box = move_slot_boxes[cur_move];
                QImage screen = env.console.video().snapshot();
                ImageMatchDetector move_slot_detector(std::move(screen), move_box, 10.0);
                
                use_move(env.console, cur_pokemon, cur_move);

                // Check if the move cannot be used due to no PP:
                screen = env.console.video().snapshot();
                if (move_slot_detector.detect(screen)){
                    // We are still on the move selection screen. No PP.
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

#ifdef DEBUG_INGO_BATTLE                    
                    std::cout << "Moved to next move " << cur_move << std::endl;
                    static int count = 0;
                    screen.save("./no_pp." + QString::number(count++) + ".png");
#endif
                }
            }

            env.update_stats();
            num_turns++;
        }
        else if (ret == 1){
            env.console.log("Transparent dialogue box.");
            
            pbf_press_button(env.console, BUTTON_B, 20, 100);
            env.console.botbase().wait_for_all_requests();
        } else if(ret == 2){
            env.console.log("Normal dialogue box.");

            pbf_press_button(env.console, BUTTON_B, 20, 100);
            env.console.botbase().wait_for_all_requests();
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

    stats.battles++;
    env.update_stats();

    return false;
}



void IngoBattleGrinder::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);

    // {
    //     // QImage image("./scripts/LA_switch_pokemon_Kuro.png");
    //     QImage image("./PLA_test_data/ingoBattle/broken_dialogue_detector.png");
    //     const bool stop_on_detected = true;
    //     NormalDialogDetector detector(env.console, env.console, stop_on_detected);
    //     bool detected = detector.process_frame(image, std::chrono::system_clock::now());
    //     std::cout << "detector " << detected << std::endl;
    //     return;
    // }

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
            throw;
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
