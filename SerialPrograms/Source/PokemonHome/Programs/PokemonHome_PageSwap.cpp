/*  Pokemon Home Page Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonHome_PageSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
    using namespace Pokemon;



PageSwap_Descriptor::PageSwap_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:PageSwap",
        STRING_POKEMON + " Home", STRING_POKEMON + " Home: Page Swap",
        "ComputerControl/blob/master/Wiki/Programs/PokemonHome/PageSwap.md",
        "Swap 30 boxes (1 page) in " + STRING_POKEMON + " Home.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



PageSwap::PageSwap()
    : DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void PageSwap::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        PokemonSwSh::resume_game_no_interact(env.console, context, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(context, BUTTON_RCLICK, 5, 5);
    }

    const uint16_t PICKUP_DELAY = 250;
    const uint16_t SCROLL_DELAY = 20;

    for (uint8_t i = 0; i < 2; i++){
        for (uint8_t j = 0; j < 3; j++){
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
            }
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(context, DPAD_LEFT, 10, SCROLL_DELAY);
            }
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_dpad(context, DPAD_DOWN, 10, SCROLL_DELAY);
        for (uint8_t j = 0; j < 3; j++){
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(context, DPAD_LEFT, 10, SCROLL_DELAY);
            }
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
            }
            pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_dpad(context, DPAD_DOWN, 10, SCROLL_DELAY);
    }
    for (uint8_t j = 0; j < 3; j++){
        pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
        for (uint8_t c = 0; c < 6; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
        pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
        pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
        for (uint8_t c = 0; c < 6; c++){
            pbf_press_dpad(context, DPAD_LEFT, 10, SCROLL_DELAY);
        }
        pbf_press_button(context, BUTTON_Y, 10, PICKUP_DELAY);
        pbf_press_dpad(context, DPAD_RIGHT, 10, SCROLL_DELAY);
    }
}






}
}
}
