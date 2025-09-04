/*  Ingo Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA_IngoMoveGrinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;




IngoMoveGrinder_Descriptor::IngoMoveGrinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:IngoMoveGrinder",
        STRING_POKEMON + " LA", "Ingo Move Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/IngoMoveGrinder.md",
        "Attend Ingo's battles to grind move related " + STRING_POKEDEX + " research tasks. "
        "More effective than Ingo Battle Grinder for " + STRING_POKEDEX + " research tasks but less effective for everything else.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class IngoMoveGrinder_Descriptor::Stats : public StatsTracker{
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
        m_display_order.emplace_back("Faint Switches", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& turns;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& faint_switches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> IngoMoveGrinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


IngoMoveGrinder::IngoMoveGrinder()
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


bool IngoMoveGrinder::start_dialog(VideoStream& stream, ProControllerContext& context){
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

bool IngoMoveGrinder::run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    IngoMoveGrinder_Descriptor::Stats& stats = env.current_stats<IngoMoveGrinder_Descriptor::Stats>();

    env.console.log("Starting battle...");

    cur_pokemon = 0;
    cur_move = 0;

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

            Move move = POKEMON_ACTIONS.get_move(cur_pokemon, cur_move);
            if (move_issued[cur_pokemon][cur_move] < move.attempts){
                // Press A to select moves
                pbf_press_button(context, BUTTON_A, 10, 125);
                context.wait_for_all_requests();
                const bool check_move_success = true;
                if (use_move(env.console, context, cur_pokemon, cur_move, move.style, check_move_success)){
                    stats.turns++;
                    if (cur_pokemon < 4)
                    {
                        stats.move_attempts++;
                        move_issued[cur_pokemon][cur_move]++;
                        env.console.log("Successfully attempted a new move " + debug_current_info() + debug_move_attempts_info());
                    }
                    context.wait_for_all_requests();
                }else{
                    pbf_press_button(context, BUTTON_B, 20, 2 * TICKS_PER_SECOND);
                    env.console.log("No PP left for pokemon " + std::to_string(cur_pokemon) + " and move " + std::to_string(cur_move));
                    if (get_next_move_to_switch_to() == 4){
                        // Press down to select pokemons
                        pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                        go_to_next_pokemon(env, context);

                    }else
{
                        go_to_next_move(env, context);
                    }
                }
            }else{
                env.console.log("Done grinding for pokemon " + std::to_string(cur_pokemon) + " and move " + std::to_string(cur_move));
                if (get_next_move_to_switch_to() == 4){
                    // Press down to select pokemons
                    pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                    go_to_next_pokemon(env, context);

                }else{
                    go_to_next_move(env, context);
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

            go_to_next_pokemon(env, context);
        }else{ // ret is 4
            env.console.log("Battle finished.");
            break;
        }
    }

    stats.battles++;
    env.update_stats();
    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            if (move_issued[i][j] < POKEMON_ACTIONS.get_move(i, j).attempts)
            {
                env.console.log("Grinding will continue." + debug_move_attempts_info());
                return false;
            }
        }
    }
    env.console.log("Grinding will stop." + debug_move_attempts_info());
    return true;
}



void IngoMoveGrinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    IngoMoveGrinder_Descriptor::Stats& stats = env.current_stats<IngoMoveGrinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);
    for (auto& row : move_issued)
    {
        for (auto& elem : row)
        {
            elem = 0;
        }
    }

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
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
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

void IngoMoveGrinder::go_to_next_move(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
{
    env.console.log("Switch to next move " + debug_current_info() + debug_move_attempts_info());
    pbf_press_button(context, BUTTON_A, 10, 125);
    size_t next_move = get_next_move_to_switch_to();
    for (size_t i = 0; i < next_move - cur_move; ++i)
    {
        pbf_press_dpad(context, DPAD_DOWN, 20, 100);
    }
    pbf_press_button(context, BUTTON_B, 10, 125);
    cur_move = next_move;
    env.console.log("Switched to next move " + debug_current_info() + debug_move_attempts_info());
    context.wait_for_all_requests();
}

void IngoMoveGrinder::go_to_next_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context)
{
    if (cur_pokemon == 4){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Abort program. Your 4 first pokemons are done grinding moves, dead or without PP. "
            "Your fifth pokemon (Arceus) died so no other choice than stopping the program.",
            env.console
        );
    }
    env.console.log("Switch to next pokemon " + debug_current_info() + debug_move_attempts_info());
    cur_pokemon = get_next_pokemon_to_switch_to();
    cur_move = 0;
    switch_pokemon(env.console, context, cur_pokemon);
    env.console.log("Switched to next pokemon " + debug_current_info() + debug_move_attempts_info());
    context.wait_for_all_requests();
}

size_t IngoMoveGrinder::get_next_move_to_switch_to() const
{
    for (size_t i = cur_move + 1; i < 4; ++i)
    {
        if (move_issued[cur_pokemon][i] < POKEMON_ACTIONS.get_move(cur_pokemon, i).attempts)
        {
            return i;
        }
    }
    // Means switch to next pokemon
    return 4;
}

size_t IngoMoveGrinder::get_next_pokemon_to_switch_to() const
{
    for (size_t i = cur_pokemon + 1; i < 4; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            if (move_issued[i][j] < POKEMON_ACTIONS.get_move(i, j).attempts)
            {
                return i;
            }
        }
    }
    // Means switch to arceus and spam moves to end the battle
    return 4;
}

std::string IngoMoveGrinder::debug_current_info() const
{
    return "(cur_pokemon : " + std::to_string(cur_pokemon) + ", cur_move : " + std::to_string(cur_move) + ")";
}

std::string IngoMoveGrinder::debug_move_attempts_info() const
{
    std::string debug = "\n";
    for (size_t i = 0; i < 4; ++i){
        for (size_t j = 0; j < 4; ++j){
            Move move = POKEMON_ACTIONS.get_move(i, j);
            if (move.attempts != 0)
            {
                debug += "(pokemon : " + std::to_string(i) + ", move : " + std::to_string(j) + " did " + std::to_string(move_issued[i][j]) + " out of " + std::to_string(move.attempts) + ")\n";
            }
        }
    }
    return debug;
}




}
}
}
