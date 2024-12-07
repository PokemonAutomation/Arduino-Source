/*  Mount Change
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA_BattleRoutines.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



void mash_A_until_end_of_battle(ConsoleHandle& console, BotBaseContext& context){
    OverworldDetector detector(console, console);
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 120 * TICKS_PER_SECOND);
        },
        {{detector}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Failed to return to overworld after 2 minutes."
        );
    }
    console.log("Returned to overworld.");
}



size_t switch_pokemon(ConsoleHandle& console, BotBaseContext& context, size_t pokemon_to_switch_to, size_t max_num_pokemon){
    if (pokemon_to_switch_to >= max_num_pokemon){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Cannot send any more Pokemon to battle, max: " + std::to_string(max_num_pokemon)
        );
    }
    // Move past leading fainted pokemon
    for(size_t i = 0; i < pokemon_to_switch_to; i++){
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
        VideoSnapshot screen = console.video().snapshot();
        if (switch_detector.process_frame(screen, current_time()) == false){
            // No longer at the switching pokemon screen
            break;
        }

        // We are still in the switching pokemon screen. So the current selected pokemon is fainted
        // and therefore cannot be used. Try the next pokemon:
        pokemon_to_switch_to++;
        if (pokemon_to_switch_to >= max_num_pokemon){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Cannot send any more Pokemon to battle, max: " + std::to_string(max_num_pokemon)
            );
        }

        // Fist hit B to clear the "cannot send pokemon" dialogue
        pbf_press_button(context, BUTTON_B, 20, 100);
        // Move to the next pokemon
        pbf_press_dpad(context, DPAD_DOWN, 20, 80);
    }

    return pokemon_to_switch_to;
}


void use_move_blindly(ConsoleHandle& console, BotBaseContext& context, MoveStyle style, size_t cur_pokemon, size_t cur_move){
    // Select move styles
    if (style == MoveStyle::Agile){
        // Agile style
        pbf_press_button(context, BUTTON_L, 10, 125);
    }else if (style == MoveStyle::Strong){
        // Strong style
        pbf_press_button(context, BUTTON_R, 10, 125);
    }

    // Use the move
    pbf_press_button(context, BUTTON_A, 10, 125);

    console.log(
        "Using pokemon " + std::to_string(cur_pokemon) + " move " + std::to_string(cur_move) +
        " style " + MoveStyle_Database().find(style)->display
    );

    pbf_wait(context, 1 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
}


bool use_move(ConsoleHandle& console, BotBaseContext& context, size_t cur_pokemon, size_t cur_move, MoveStyle style, bool check_success){
    if (check_success == false){
        use_move_blindly(console, context, style, cur_pokemon, cur_move);
        return true;
    }

    // The location of the move slots when choosing which move to use during battle.
    // These boxes will be used to check whether the content in those boxes are changed or not
    // after selecting one move to use. In this way we can detect whether the move is out of PP.
    const ImageFloatBox move_slot_boxes[4] = {
        {0.6600, 0.6220, 0.2500, 0.0320},
        {0.6395, 0.6875, 0.2500, 0.0320},
        {0.6190, 0.7530, 0.2500, 0.0320},
        {0.5985, 0.8185, 0.2500, 0.0320},
    };

    VideoSnapshot screen = console.video().snapshot();
    ImageMatchDetector move_slot_detector(std::move(screen.frame), move_slot_boxes[cur_move], 10.0);

    use_move_blindly(console, context, style, cur_pokemon, cur_move);

    screen = console.video().snapshot();

    const bool still_on_move_screen = move_slot_detector.detect(screen);

#ifdef DEBUG_NO_PP
    if (still_on_move_screen == false){
        static int count = 0;
        screen.save("./no_pp." + std::to_string(count++) + ".png");
    }
#endif

    return still_on_move_screen == false;
}

void use_next_move_with_pp(ConsoleHandle& console, BotBaseContext& context, size_t cur_pokemon, size_t& cur_move){
    const bool check_move_success = true;
    while (use_move(console, context, cur_pokemon, cur_move, MoveStyle::NoStyle, check_move_success) == false){
        // We are still on the move selection screen. No PP.
        if (cur_move == 3){
            // Pokemon has zero PP on all moves. This should not happen as it will just use
            // Struggle.
            console.log("No PP on all moves. Abort program.", COLOR_RED);
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "No PP on all moves."
            );
        }
        
        // Go to the next move.
        pbf_press_dpad(context, DPAD_DOWN, 20, 100);
        // env.console.context().wait_for_all_requests();
        cur_move++;
        console.log("No PP. Use next move, " + std::to_string(cur_move), COLOR_RED);
    }
}




}
}
}
