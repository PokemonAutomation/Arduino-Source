/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void egg_spin(ProControllerContext& context, Milliseconds duration){
    for (Milliseconds c = 0ms; c < duration; c += 42*8ms){
        pbf_move_left_joystick(context, 0, 0, 5, 0);
        pbf_move_left_joystick(context, 128, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 128, 5, 0);
        pbf_move_left_joystick(context, 255, 255, 5, 0);
        pbf_move_left_joystick(context, 128, 255, 5, 0);
        pbf_move_left_joystick(context, 0, 255, 6, 0);
        pbf_move_left_joystick(context, 0, 128, 6, 0);
    }
}
void egg_spin_with_A(ProControllerContext& context, Milliseconds duration){
    for (Milliseconds c = 0ms; c < duration; c += 42*8ms){
        ssf_press_button(context, BUTTON_ZL, 0ms, 80ms);
        pbf_move_left_joystick(context, 0, 0, 5, 0);
        pbf_move_left_joystick(context, 128, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 0, 5, 0);
        pbf_move_left_joystick(context, 255, 128, 5, 0);
        pbf_move_left_joystick(context, 255, 255, 5, 0);
        pbf_move_left_joystick(context, 128, 255, 5, 0);
        pbf_move_left_joystick(context, 0, 255, 6, 0);
        pbf_move_left_joystick(context, 0, 128, 6, 0);
    }
}

void pickup_column(ProControllerContext& context){
//    const uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    for (size_t c = 0; c < 30; c++){
        ssf_issue_scroll(context, DPAD_DOWN, 24ms);
    }
    pbf_press_button(context, BUTTON_ZL, 160ms, GameSettings::instance().BOX_PICKUP_DROP_DELAY0);
}
void party_to_column(ProControllerContext& context, uint8_t column){
    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    pbf_move_right_joystick(context, 128, 0, 160ms, BOX_SCROLL_DELAY);
    if (column < 3){
        for (uint8_t c = 0; c <= column; c++){
            pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
        }
    }else{
        for (uint8_t c = 6; c != column; c--){
            pbf_move_right_joystick(context, 0, 128, 160ms, BOX_SCROLL_DELAY);
        }
    }
}
void column_to_party(ProControllerContext& context, uint8_t column){
    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    if (column < 3){
        for (uint8_t c = 0; c <= column; c++){
            pbf_move_right_joystick(context, 0, 128, 160ms, BOX_SCROLL_DELAY);
        }
    }else{
        for (uint8_t c = 6; c != column; c--){
            pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
        }
    }
    pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
}

void withdraw_1st_column_from_overworld(ProControllerContext& context){
    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;
    overworld_to_box(context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pickup_column(context);
    pbf_move_right_joystick(context, 0, 128, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
    pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(context);
}
void deposit_party_to_column(ProControllerContext& context, uint8_t column){
    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;
    overworld_to_box(context);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_press_button(context, BUTTON_Y, 20, 50);
    pbf_move_right_joystick(context, 0, 128, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);

    //  Deposit current column.
    pickup_column(context);
    party_to_column(context, column);
    pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
}
void swap_party(ProControllerContext& context, uint8_t current_column){
    deposit_party_to_column(context, current_column);

    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;

    pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
    if (current_column < 5){
        pickup_column(context);
        column_to_party(context, current_column + 1);
    }else{
        pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().BOX_CHANGE_DELAY0);
        pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
        pickup_column(context);
        column_to_party(context, 0);
    }
    pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
    box_to_overworld(context);
}







}
}
}
