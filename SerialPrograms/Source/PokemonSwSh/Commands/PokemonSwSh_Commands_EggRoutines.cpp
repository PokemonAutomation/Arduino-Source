/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_EggRoutines.h"
//#include "PokemonSwSh_Messages_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop(SwitchControllerContext& context){
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
    ssf_press_left_joystick(context, STICK_MAX, 160, 0, 510);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 390);
    ssf_mash1_button(context, BUTTON_B, 390);

    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
}
void move_while_mashing_B(SwitchControllerContext& context, uint16_t duration){
    //  Hold the joystick to the right for the entire duration.
    ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0, duration);

    //  While the above is running, spam B.
    ssf_mash1_button(context, BUTTON_B, duration);
}
void spin_and_mash_A(SwitchControllerContext& context, uint16_t duration){
    for (uint16_t c = 0; c < duration; c += 128){
        ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_CENTER, STICK_MIN, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);
    }
}
void travel_to_spin_location(SwitchControllerContext& context){
    ssf_press_left_joystick(context, STICK_MAX, 144, 100, 250);
    ssf_press_button(context, BUTTON_A, 100);
    ssf_press_button(context, BUTTON_A, 50);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
}
void travel_back_to_lady(SwitchControllerContext& context){
    ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 30, 30);
    ssf_press_left_joystick(context, STICK_MAX, 144, 260, 260);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 400);
    ssf_mash1_button(context, BUTTON_B, 400);
    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
}



}
}

