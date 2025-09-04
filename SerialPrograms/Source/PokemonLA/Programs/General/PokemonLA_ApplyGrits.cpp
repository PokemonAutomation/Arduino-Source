/*  Apply Grits
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_ApplyGrits.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;

namespace{

// For each grit item, how many levels it can increase:
const size_t grit_level_increase[4] = {3, 3, 3, 1};

}


ApplyGrits_Descriptor::ApplyGrits_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:ApplyGrits",
        STRING_POKEMON + " LA", "Apply Grits",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ApplyGrits.md",
        "Use Grits items on " + STRING_POKEMON,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


ApplyGrits::ApplyGrits()
    : NUM_POKEMON(
        "<b>Number of " + STRING_POKEMON + " to Apply Grits</b>:",
        {
            {1, "1", "1"},
            {2, "2", "2"},
            {3, "3", "3"},
            {4, "4", "4"},
            {5, "5", "5"},
            {6, "6", "6"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        1
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(NUM_POKEMON);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ApplyGrits::ApplyGritsOnOnePokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t pokemon_index){
    // Start the function when the game is in the item menu, with cursor hovering over Grit Dust
    // Grit Gravel, Grit Pebble and Grit Rock must be on the right side of Grit Dust, in the correct order.

    for(size_t grit_index = 0; grit_index < 4; grit_index++){
        
        if (grit_index > 0){
            // Move to the next Grit item
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
        }

        // Press A to open the item menu of Grit Dust
        pbf_press_button(context, BUTTON_A, 20, 50);
        // Press A again to use the item
        pbf_press_button(context, BUTTON_A, 20, 50);

        // Move down the pokemon list
        for(size_t i = 0; i < pokemon_index; i++){
            pbf_press_dpad(context, DPAD_DOWN, 20, 80);
        }

        // Select the pokemon
        pbf_press_button(context, BUTTON_A, 20, 80);

        // For each of the size pokemon attributes: HP, Attack, Defense, Sp. Atk, Sp. Def, Speed
        for(size_t attr_index = 0; attr_index < 6; attr_index++){
            if (attr_index > 0){
                // Move to the next attribute
                pbf_press_dpad(context, DPAD_DOWN, 20, 50);
            }
            for(size_t i = 0; i < grit_level_increase[grit_index]; i++){
                // One press to apply the Grit item
                pbf_press_button(context, BUTTON_A, 20, 100);
                // Second press to clear the message box, whether it is applied successfully or not.
                pbf_press_button(context, BUTTON_A, 20, 100);
            }
        }

        // Press B twice to back to item selection
        pbf_press_button(context, BUTTON_B, 20, 50);
        pbf_press_button(context, BUTTON_B, 20, 50);
    }

    // Move back to hovering over Grit Dust
    pbf_press_dpad(context, DPAD_LEFT, 20, 50);
    pbf_press_dpad(context, DPAD_LEFT, 20, 50);
    pbf_press_dpad(context, DPAD_LEFT, 20, 50);

    context.wait_for_all_requests();
}

void ApplyGrits::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    size_t num_pokemon = NUM_POKEMON.current_value();
    for(size_t i = 0; i < num_pokemon; i++){
        ApplyGritsOnOnePokemon(env, context, i);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
