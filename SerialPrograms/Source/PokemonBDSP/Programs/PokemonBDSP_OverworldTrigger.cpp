/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP_OverworldTrigger.h"
#include "PokemonBDSP_GameNavigation.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



OverworldTrigger::OverworldTrigger()
    : GroupOption("Trigger Method", LockMode::LOCK_WHILE_RUNNING)
    , TRIGGER_METHOD(
        "<b>Maneuver:</b><br>How to trigger an encounter",
        {
            {TriggerMethod::HORIZONTAL_NO_BIAS,     "horizontal-none",  "Move left/right. (no bias)"},
            {TriggerMethod::HORIZONTAL_BIAS_LEFT,   "horizontal-left",  "Move left/right. (bias left)"},
            {TriggerMethod::HORIZONTAL_BIAS_RIGHT,  "horizontal-right", "Move left/right. (bias right)"},
            {TriggerMethod::VERTICAL_NO_BIAS,       "vertical-none",    "Move up/down. (no bias)"},
            {TriggerMethod::VERTICAL_BIAS_UP,       "vertical-up",      "Move up/down. (bias up)"},
            {TriggerMethod::VERTICAL_BIAS_DOWN,     "vertical-down",    "Move up/down. (bias down)"},
            {TriggerMethod::SWEET_SCENT,            "sweet-scent",      "Sweet scent"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TriggerMethod::HORIZONTAL_NO_BIAS
    )
    , MOVE_DURATION(
        "<b>Move Duration:</b><br>Move in each direction for this long before turning around.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "1 * TICKS_PER_SECOND"
    )
    , SWEET_SCENT_POKEMON_LOCATION(
        "<b>Sweet Scent Pokemon Location:</b><br>Which Pokemon in the party knows Sweet Scent.",
        {
            {0, "slot1", "1st"},
            {1, "slot2", "2nd"},
            {2, "slot3", "3rd"},
            {3, "slot4", "4th"},
            {4, "slot5", "2nd last"},
            {5, "slot6", "Last"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MOVE_DURATION);
    PA_ADD_OPTION(SWEET_SCENT_POKEMON_LOCATION);
}


void OverworldTrigger::run_trigger(BotBaseContext& context) const{
    switch (TRIGGER_METHOD){
    case TriggerMethod::HORIZONTAL_NO_BIAS:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 0, 128, 128, 128, MOVE_DURATION);
//        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case TriggerMethod::HORIZONTAL_BIAS_LEFT:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 0, 128, 128, 128, MOVE_DURATION + 25);
//        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case TriggerMethod::HORIZONTAL_BIAS_RIGHT:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 255, 128, 128, 128, MOVE_DURATION + 25);
//        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        break;
    case TriggerMethod::VERTICAL_NO_BIAS:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, MOVE_DURATION);
//        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case TriggerMethod::VERTICAL_BIAS_UP:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, MOVE_DURATION + 25);
//        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case TriggerMethod::VERTICAL_BIAS_DOWN:
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 255, 128, 128, MOVE_DURATION + 25);
//        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        break;
    default:;
    }
}

bool OverworldTrigger::find_encounter(ConsoleHandle& console, BotBaseContext& context) const{
    BattleMenuWatcher battle_menu_detector(BattleType::STANDARD);
    StartBattleDetector start_battle_detector(console);

    int result = 0;
    if (TRIGGER_METHOD != TriggerMethod::SWEET_SCENT){
        //  Move character back and forth to trigger encounter.
        result = run_until(
            console, context,
            [&](BotBaseContext& context){
                while (true){
                    run_trigger(context);
                }
            },
            {
                {battle_menu_detector},
                {start_battle_detector},
            }
        );
    }else{
        console.overlay().add_log("Using Sweet Scent", COLOR_CYAN);
        //  Use Sweet Scent to trigger encounter.
        overworld_to_menu(console, context);

        //  Go to pokemon page
        const uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
        pbf_press_button(context, BUTTON_ZL, 20, MENU_TO_POKEMON_DELAY);

        //  Go to the pokemon that knows Sweet Scent
        const size_t location = SWEET_SCENT_POKEMON_LOCATION.current_value();
        const uint16_t change_pokemon_delay = 20;
        if (location >= 1 && location <= 3){
            const size_t move_down_times = location;
            for(size_t i = 0; i < move_down_times; ++i){
                pbf_press_dpad(context, DPAD_DOWN, 20, change_pokemon_delay);
            }
        }else if (location >= 1){ // for location 4 and 5
            const size_t move_down_times = 6 - location;
            for (size_t i = 0; i < move_down_times; ++i){
                pbf_press_dpad(context, DPAD_UP, 20, change_pokemon_delay);
            }
        }

        //  Open the pokemon menu of the selected pokemon
        const uint16_t pokemon_to_pokemon_menu_delay = 30;
        pbf_press_button(context, BUTTON_ZL, 20, pokemon_to_pokemon_menu_delay);
        //  Move down one menuitem to select "Sweet Scent"
        const uint16_t move_pokemon_menu_item_delay = 30;
        pbf_press_dpad(context, DPAD_DOWN, 20, move_pokemon_menu_item_delay);
        //  Use sweet scent
        pbf_mash_button(context, BUTTON_ZL, 30);

        result = wait_until(
            console, context, std::chrono::seconds(30),
            {
                {battle_menu_detector},
                {start_battle_detector},
            }
        );
        if (result < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Battle not detected after Sweet Scent for 30 seconds."
            );
        }
    }

    switch (result){
    case 0:
        console.log("Unexpected Battle.", COLOR_RED);
        return false;
    case 1:
        console.log("Battle started!");
        return true;
    }
    return false;
}


}
}
}
