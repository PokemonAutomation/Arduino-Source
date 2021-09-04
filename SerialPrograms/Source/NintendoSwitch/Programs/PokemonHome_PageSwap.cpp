/*  Pokemon Home Page Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonHome_PageSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



PageSwap_Descriptor::PageSwap_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonHome:PageSwap",
        STRING_POKEMON + " Home: Page Swap",
        "SwSh-Arduino/wiki/Advanced:-PkmnHomePageSwap",
        "Swap 30 boxes (1 page) in " + STRING_POKEMON + " Home.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



PageSwap::PageSwap(const PageSwap_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void PageSwap::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(env.console, BUTTON_RCLICK, 5, 5);
    }

    const uint16_t PICKUP_DELAY = 50;
    const uint16_t SCROLL_DELAY = 20;

    for (uint8_t i = 0; i < 2; i++){
        for (uint8_t j = 0; j < 3; j++){
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
            }
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(env.console, DPAD_LEFT, 10, SCROLL_DELAY);
            }
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_dpad(env.console, DPAD_DOWN, 10, SCROLL_DELAY);
        for (uint8_t j = 0; j < 3; j++){
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(env.console, DPAD_LEFT, 10, SCROLL_DELAY);
            }
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            for (uint8_t c = 0; c < 6; c++){
                pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
            }
            pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_dpad(env.console, DPAD_DOWN, 10, SCROLL_DELAY);
    }
    for (uint8_t j = 0; j < 3; j++){
        pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
        for (uint8_t c = 0; c < 6; c++){
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
        }
        pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
        pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
        pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
        for (uint8_t c = 0; c < 6; c++){
            pbf_press_dpad(env.console, DPAD_LEFT, 10, SCROLL_DELAY);
        }
        pbf_press_button(env.console, BUTTON_Y, 10, PICKUP_DELAY);
        pbf_press_dpad(env.console, DPAD_RIGHT, 10, SCROLL_DELAY);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}






}
}
}
