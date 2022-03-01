/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
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
    : GroupOption("Trigger Method")
    , TRIGGER_METHOD(
        "<b>Maneuver:</b><br>How to trigger an encounter",
        {
            "Move left/right. (no bias)",
            "Move left/right. (bias left)",
            "Move left/right. (bias right)",
            "Move up/down. (no bias)",
            "Move up/down. (bias up)",
            "Move up/down. (bias down)",
            "Sweet scent",
        }, 0
    )
    , MOVE_DURATION(
        "<b>Move Duration:</b><br>Move in each direction for this long before turning around.",
        "1 * TICKS_PER_SECOND"
    )
    , SWEET_SCENT_POKEMON_LOCATION(
        "<b>Sweet Scent Pokemon Location:</b><br>Which Pokemon in the party knows Sweet Scent.",
        {
          "1st",
          "2nd",
          "3rd",
          "4th",
          "2nd last",
          "Last",
        }, 0
    )
{
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MOVE_DURATION);
    PA_ADD_OPTION(SWEET_SCENT_POKEMON_LOCATION);
}


void OverworldTrigger::run_trigger(const BotBaseContext& context) const{
    switch (TRIGGER_METHOD){
    case 0:
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case 1:
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case 2:
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        break;
    case 3:
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case 4:
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case 5:
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        break;
    }
}

bool OverworldTrigger::find_encounter(SingleSwitchProgramEnvironment& env) const{
    BattleMenuWatcher battle_menu_detector(BattleType::STANDARD);
    StartBattleDetector start_battle_detector(env.console);

    int result = 0;
    if (TRIGGER_METHOD < 6){
        //  Move character back and forth to trigger encounter.
        result = run_until(
            env, env.console,
            [&](const BotBaseContext& context){
                while (true) {
                    run_trigger(context);
                }
            },
            {
                &battle_menu_detector,
                &start_battle_detector,
            }
        );
    }else{
        //  Use Sweet Scent to trigger encounter.
        overworld_to_menu(env, env.console);

        //  Go to pokemon page
        const uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
        pbf_press_button(env.console, BUTTON_ZL, 20, MENU_TO_POKEMON_DELAY);

        //  Go to the pokemon that knows Sweet Scent
        const size_t location = SWEET_SCENT_POKEMON_LOCATION;
        const uint16_t change_pokemon_delay = 20;
        if (location >= 1 && location <= 3){
            const size_t move_down_times = location;
            for(size_t i = 0; i < move_down_times; ++i){
                pbf_press_dpad(env.console, DPAD_DOWN, 1, change_pokemon_delay);
            }
        }else if (location >= 1){ // for location 4 and 5
            const size_t move_down_times = 6 - location;
            for (size_t i = 0; i < move_down_times; ++i){
                pbf_press_dpad(env.console, DPAD_UP, 1, change_pokemon_delay);
            }
        }

        //  Open the pokemon menu of the selected pokemon
        const uint16_t pokemon_to_pokemon_menu_delay = 30;
        pbf_press_button(env.console, BUTTON_ZL, 20, pokemon_to_pokemon_menu_delay);
        //  Move down one menuitem to select "Sweet Scent"
        const uint16_t move_pokemon_menu_item_delay = 30;
        pbf_press_dpad(env.console, DPAD_DOWN, 1, move_pokemon_menu_item_delay);
        //  Use sweet scent
        pbf_mash_button(env.console, BUTTON_ZL, 30);

        result = wait_until(
            env, env.console, std::chrono::seconds(30),
            { &battle_menu_detector, &start_battle_detector }
        );
        if (result < 0){
            throw OperationFailedException(env.console, "Battle not detected after Sweet Scent for 30 seconds.");
        }
    }

    switch (result){
    case 0:
        env.console.log("Unexpected Battle.", COLOR_RED);
        return false;
    case 1:
        env.console.log("Battle started!");
        return true;
    }
    return false;
}


}
}
}
