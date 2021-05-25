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



PageSwap::PageSwap()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        STRING_POKEMON + " Home - Page Swap",
        "SerialPrograms/PokemonHome-PageSwap.md",
        "Swap 30 boxes (1 page) in " + STRING_POKEMON + " Home."
    )
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        false
    )
{
    m_options.emplace_back(&DODGE_SYSTEM_UPDATE_WINDOW, "DODGE_SYSTEM_UPDATE_WINDOW");
}

void PageSwap::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);
    resume_game_no_interact(env.console, DODGE_SYSTEM_UPDATE_WINDOW);

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
