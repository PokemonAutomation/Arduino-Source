/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_EggRoutines.h"
//#include "PokemonSwSh_Messages_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop(ProControllerContext& context){
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
    ssf_press_left_joystick(context, STICK_MAX, 160, 0, 510);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 390);
    ssf_mash1_button(context, BUTTON_B, 90);
    ssf_do_nothing(context, 300);

    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
}
void move_while_mashing_B(ProControllerContext& context, Milliseconds duration){
    //  Hold the joystick to the right for the entire duration.
    ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0ms, duration);

    //  While the above is running, spam B.
    ssf_mash1_button(context, BUTTON_B, duration);
}
void spin_and_mash_A(ProControllerContext& context, Milliseconds duration){
    for (Milliseconds c = 0ms; c < duration; c += 1280ms){
        ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, STICK_CENTER, STICK_MIN, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);
    }
}
void travel_to_spin_location(ProControllerContext& context){
    ssf_press_left_joystick(context, STICK_MAX, 144, 100, 250);
    ssf_press_button(context, BUTTON_A, 800ms);
    ssf_press_button(context, BUTTON_A, 400ms);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
}
void travel_back_to_lady(ProControllerContext& context){
    ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 30, 30);
    ssf_press_left_joystick(context, STICK_MAX, 144, 260, 260);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 400);
    ssf_mash1_button(context, BUTTON_B, 100);
    ssf_do_nothing(context, 300);
    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
}



}
}

