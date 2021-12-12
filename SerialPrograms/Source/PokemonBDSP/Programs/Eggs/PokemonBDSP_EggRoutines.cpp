/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void egg_spin(const BotBaseContext& context, uint16_t duration){
    for (uint16_t c = 0; c < duration; c += 41){
#if 0
        pbf_move_left_joystick(context, 128, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 128, 5, 0);
        pbf_move_left_joystick(context, 255, 255, 5, 0);
        pbf_move_left_joystick(context, 128, 255, 5, 0);
        pbf_move_left_joystick(context, 0, 255, 5, 0);
        pbf_move_left_joystick(context, 0, 128, 6, 0);
        pbf_move_left_joystick(context, 0, 0, 5, 0);
#endif
        pbf_controller_state(context, BUTTON_ZL, DPAD_NONE, 128, 0, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 255, 0, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 255, 128, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 255, 255, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 128, 255, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 0, 255, 128, 128, 5);
        pbf_controller_state(context, 0, DPAD_NONE, 0, 128, 128, 128, 6);
        pbf_controller_state(context, 0, DPAD_NONE, 0, 0, 128, 128, 5);
    }
}

void pickup_column(const BotBaseContext& context){
//    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    for (size_t c = 0; c < 10; c++){
        pbf_move_right_joystick(context, 128, 255, 5, 3);
    }
    pbf_press_button(context, BUTTON_ZL, 20, GameSettings::instance().BOX_PICKUP_DROP_DELAY);
}
void party_to_column(const BotBaseContext& context, uint8_t column){
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    pbf_move_right_joystick(context, 128, 0, 20, BOX_SCROLL_DELAY);
    if (column < 3){
        for (uint8_t c = 0; c <= column; c++){
            pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
        }
    }else{
        for (uint8_t c = 6; c != column; c--){
            pbf_move_right_joystick(context, 0, 128, 20, BOX_SCROLL_DELAY);
        }
    }
}
void column_to_party(const BotBaseContext& context, uint8_t column){
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    if (column < 3){
        for (uint8_t c = 0; c <= column; c++){
            pbf_move_right_joystick(context, 0, 128, 20, BOX_SCROLL_DELAY);
        }
    }else{
        for (uint8_t c = 6; c != column; c--){
            pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
        }
    }
    pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
}

void overworld_to_box(const BotBaseContext& context){
    pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
//    pbf_press_button(context, BUTTON_ZL, 20, GameSettings::instance().MENU_TO_POKEMON_DELAY);

    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
    pbf_mash_button(context, BUTTON_ZL, 30);
    if (MENU_TO_POKEMON_DELAY > 30){
        pbf_wait(context, MENU_TO_POKEMON_DELAY - 30);
    }

    pbf_press_button(context, BUTTON_R, 20, GameSettings::instance().POKEMON_TO_BOX_DELAY0);
}
void box_to_overworld(const BotBaseContext& context){
    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().BOX_TO_POKEMON_DELAY);
    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().POKEMON_TO_MENU_DELAY);
    pbf_press_button(context, BUTTON_B, 20, GameSettings::instance().MENU_TO_OVERWORLD_DELAY);
}
void withdraw_1st_column_from_overworld(const BotBaseContext& context){
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    const uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    overworld_to_box(context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pickup_column(context);
    pbf_move_right_joystick(context, 0, 128, 20, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
    pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(context);
}
void deposit_party_to_column(const BotBaseContext& context, uint8_t column){
    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    const uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    overworld_to_box(context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_move_right_joystick(context, 0, 128, 20, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);

    //  Deposit current column.
    pickup_column(context);
    party_to_column(context, column);
    pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
}
void swap_party(const BotBaseContext& context, uint8_t current_column){
    deposit_party_to_column(context, current_column);

    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    const uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;

    pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
    if (current_column < 5){
        pickup_column(context);
        column_to_party(context, current_column + 1);
    }else{
        pbf_press_button(context, BUTTON_R, 20, GameSettings::instance().BOX_CHANGE_DELAY_0);
        pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
        pickup_column(context);
        column_to_party(context, 0);
    }
    pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(context);
}







}
}
}
