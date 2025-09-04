/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
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
#include "PokemonLA_IngoBattleGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


// #define DEBUG_INGO_BATTLE



IngoBattleGrinder_Descriptor::IngoBattleGrinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:IngoBattleGrinder",
        STRING_POKEMON + " LA", "Ingo Battle Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/IngoBattleGrinder.md",
        "Attend Ingo's battles to grind exp and move related " + STRING_POKEDEX + " research tasks. "
        "Less effective than Ingo Move Grinder for " + STRING_POKEDEX + " research tasks but more effective for everything else.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class IngoBattleGrinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : battles(m_stats["Battles"])
        , turns(m_stats["Turns"])
        , lead_move_attempts(m_stats["Lead Move Attempts"])
        , faint_switches(m_stats["Faint Switches"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Turns");
        m_display_order.emplace_back("Lead Move Attempts");
        m_display_order.emplace_back("Faint Switches", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& turns;
    std::atomic<uint64_t>& lead_move_attempts;
    std::atomic<uint64_t>& faint_switches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> IngoBattleGrinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


IngoBattleGrinder::IngoBattleGrinder()
    : NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
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



bool IngoBattleGrinder::start_dialog(VideoStream& stream, ProControllerContext& context){
    // First press A to start talking with Ingo.
    pbf_press_button(context, BUTTON_A, 20, 150);
    context.wait_for_all_requests();

    {
        ButtonDetector button0(
            stream.logger(), stream.overlay(),
            ButtonType::ButtonA,
            {0.50, 0.408, 0.40, 0.042},
            std::chrono::milliseconds(100),
            true
        );
        ButtonDetector button1(
            stream.logger(), stream.overlay(),
            ButtonType::ButtonA,
            {0.50, 0.450, 0.40, 0.042},
            std::chrono::milliseconds(100),
            true
        );
        ButtonDetector button2(
            stream.logger(), stream.overlay(),
            ButtonType::ButtonA,
            {0.50, 0.492, 0.40, 0.042},
            std::chrono::milliseconds(100),
            true
        );
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                for (size_t c = 0; c < 10; c++){
                    pbf_press_button(context, BUTTON_A, 20, 150);
                }
            },
            {
                {button0},
                {button1},
                {button2},
            }
        );
        switch (ret){
        case 0:
            //  Version 1.1 without new options unlocked.
            return false;
        case 1:
            //  Version 1.0
            return true;
        case 2:
            //  Version 1.1 with new options unlocked.
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to detect options after 10 A presses.",
                stream
            );
        }
    }

    pbf_press_button(context, BUTTON_A, 20, 150);
    context.wait_for_all_requests();

    ButtonDetector button2(
        stream.logger(), stream.overlay(),
        ButtonType::ButtonA,
        {0.50, 0.350, 0.40, 0.400},
        std::chrono::milliseconds(100),
        true
    );
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            for (size_t c = 0; c < 5; c++){
                pbf_press_button(context, BUTTON_A, 20, 150);
            }
        },
        {{button2}}
    );
    switch (ret){
    case 0:
        return false;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to find opponent list options after 5 A presses.",
            stream
        );
    }
}


bool IngoBattleGrinder::run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::map<size_t, size_t>& pokemon_move_attempts){
    IngoBattleGrinder_Descriptor::Stats& stats = env.current_stats<IngoBattleGrinder_Descriptor::Stats>();

    env.console.log("Starting battle...");

    // Talk to Ingo to start conversation and select regular battles:
    // The dialogues are different between version 10 (the vanilla version) and later versions.
    bool version_10 = start_dialog(env.console, context);
    env.log(std::string("Detected current version: ") + (version_10 ? "1.0" : "1.2"));

    IngoOpponentMenuLocation menu_location = version_10
        ? INGO_OPPONENT_MENU_LOCATIONS_V10[OPPONENT.current_value()]
        : INGO_OPPONENT_MENU_LOCATIONS_V12[OPPONENT.current_value()];

    // Choose which opponent
    if (menu_location.page < 0){
        throw UserSetupError(env.console, "Opponent doesn't exist in this version of the game.");
    }

    //  Move to page.
    for (int8_t c = 0; c < menu_location.page; c++){
        pbf_press_dpad(context, DPAD_UP, 10, 60);
        pbf_press_dpad(context, DPAD_UP, 10, 60);
        pbf_press_button(context, BUTTON_A, 10, 100);
    }

    //  Move to slot.
    for (int8_t c = 0; c < menu_location.index; c++){
        pbf_press_dpad(context, DPAD_DOWN, 10, 60);
    }

    // Press the button to select the opponent
    pbf_press_button(context, BUTTON_A, 10, 115);
    pbf_wait(context, 1 * TICKS_PER_SECOND);
    context.wait_for_all_requests();

    env.log("Finish selecting the opponent. Battle should start now.");

    // Which move (0, 1, 2 or 3) to use in next turn.
    size_t cur_move = 0;
    // The battle-order index of the current pokemon on the battle field.
    // This index starts at 0. Whenever a new pokemon is sent to battle, the index adds by 1.
    // It is equal to how many pokemon have left the battle.
    // Note: battle order is different than party order, which is the order of the pokemon in the party.
    // When the player selects a pokemon in the lower right corner of the screen in the overworld using "L"
    // and "R" buttons, this pokemon will be sent to the battle first, with battle-order index of 0. But this
    // pokemon can be in any place in the party list, therefore can have any party-order index.
    size_t cur_pokemon = 0;
    // How many turns have passed for the current pokemon in this battle.
    // This turn count is reset to zero when a new pokemon is sent to battle.
    size_t num_turns = 0;
    // Used to skip fainted pokemon in the party when switching a pokemon
    // This is the party-order index of the pokemon to switch to.
    // The index is the index in the pokemon party list.
    size_t next_pokemon_to_switch_to = 0;

    // Whether this is the last battle. After the last battle ends, stops the program.
    bool last_battle = false;

    // Switch pokemon and update the battle states:
    auto switch_cur_pokemon = [&](){
        cur_move = 0;
        num_turns = 0;
        next_pokemon_to_switch_to = switch_pokemon(env.console, context, next_pokemon_to_switch_to);
        next_pokemon_to_switch_to++;
        cur_pokemon++;
    };

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
            env.console, context, std::chrono::minutes(2),
            {
                {battle_menu_detector},
                {dialogue_ellipse_detector},
                {normal_dialogue_detector},
                {pokemon_switch_detector},
                {arc_phone_detector},
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to find battle menu after 2 minutes.",
                env.console
            );
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);
            stats.turns++;

            // User may want to switch the pokemon after some turns, to get more exp, or prevent if from
            // fainting.
            if (POKEMON_ACTIONS.switch_pokemon(cur_pokemon, num_turns)){
                env.console.log("Switch pokemon");

                // Go to the switching pokemon screen:
                pbf_press_dpad(context, DPAD_DOWN, 20, 100);

                switch_cur_pokemon();
            }else{
                // Choose move to use!
                if (cur_pokemon == 0){
                    // We collect the stat of move attempts of the first pokemon.
                    stats.lead_move_attempts++;
                }

                // Press A to select moves
                pbf_press_button(context, BUTTON_A, 10, 125);
                context.wait_for_all_requests();

                // Use move. 
                // Use while loop to go to next move if the current move has no PP.
                // No PP is detected by checking whether the pixels on the selected move menu item is still the same
                // after about one second.
                // Note: if the pokemon has no PP on any moves and results to Struggle, the fight animation should
                // ensure we won't get the same pixels on the area that the move menu item would appear, so we won't
                // get stuck in this while loop.
                MoveStyle style = POKEMON_ACTIONS.get_style(cur_pokemon, cur_move);
                const bool check_move_success = true;
                while (use_move(env.console, context, cur_pokemon, cur_move, style, check_move_success) == false){
                    // We are still on the move selection screen. No PP.
                    if (cur_move == 3){
                        // Pokemon has zero PP on all moves. This should not happen as it will just use
                        // Struggle.
                        env.console.log("No PP on all moves. Abort program.", COLOR_RED);
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "No PP on all moves.",
                            env.console
                        );
                    }
                    
                    // Go to the next move.
                    pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                    // env.console.context().wait_for_all_requests();
                    cur_move++;
                    env.console.log("No PP. Use next move, " + std::to_string(cur_move), COLOR_RED);
                    
                    style = POKEMON_ACTIONS.get_style(cur_pokemon, cur_move);
                }

                num_turns++;
                pokemon_move_attempts[cur_pokemon]++;
                // Check whether to stop battle
                if (last_battle == false){
                    last_battle = POKEMON_ACTIONS.stop_battle(cur_pokemon, pokemon_move_attempts[cur_pokemon]);
                    if (last_battle){
                        env.log("Target move attempts reached: " + std::to_string(pokemon_move_attempts[cur_pokemon]) +
                            ". Stop program after this battle finishes.");
                    }
                }
            }

            env.update_stats();
        }else if (ret == 1){
            env.console.log("Transparent dialogue box.");
            
            pbf_press_button(context, BUTTON_B, 20, 100);
            context.wait_for_all_requests();
        }else if(ret == 2){
            env.console.log("Normal dialogue box.");

            pbf_press_button(context, BUTTON_B, 20, 100);
            context.wait_for_all_requests();
        }else if (ret == 3){
            env.console.log("Pokemon fainted.", COLOR_RED);
            stats.faint_switches++;
            env.update_stats();

            switch_cur_pokemon();
        }else{ // ret is 4
            env.console.log("Battle finished.");
            break;
        }
    }

    stats.battles++;
    env.update_stats();

    return last_battle;
}



void IngoBattleGrinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    IngoBattleGrinder_Descriptor::Stats& stats = env.current_stats<IngoBattleGrinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    // {
    //     // ImageRGB32 image("./scripts/LA_switch_pokemon_Kuro.png");
    //     ImageRGB32 image("./PLA_test_data/ingoBattle/broken_dialogue_detector.png");
    //     const bool stop_on_detected = true;
    //     NormalDialogDetector detector(env.console, env.console, stop_on_detected);
    //     bool detected = detector.process_frame(image, current_time());
    //     std::cout << "detector " << detected << std::endl;
    //     return;
    // }

    // pokemon index -> number of move attempts made so far
    std::map<size_t, size_t> pokemon_move_attempts;

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            if (run_iteration(env, context, pokemon_move_attempts)){
                break;
            }
        }catch (OperationFailedException&){
            stats.errors++;
            throw;
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
