/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_EggRoutines.h"
//#include "PokemonSwSh_Messages_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop(ProControllerContext& context){
    ssf_press_left_joystick(context, {+1, -1}, 400ms, 400ms);
    ssf_press_left_joystick(context, {+1, -0.252}, 0ms, 4080ms);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_button(context, BUTTON_A, 1360ms);
    ssf_press_left_joystick(context, {-1, 0}, 400ms, 3120ms);
    ssf_mash1_button(context, BUTTON_B, 720ms);
    ssf_do_nothing(context, 2400ms);

    ssf_press_left_joystick(context, {+0.5, +1}, 960ms, 960ms);
    ssf_press_left_joystick(context, {+1, +1}, 960ms, 960ms);
}
void move_while_mashing_B(ProControllerContext& context, Milliseconds duration){
    //  Hold the joystick to the right for the entire duration.
    ssf_press_left_joystick(context, {+1, 0}, 0ms, duration);

    //  While the above is running, spam B.
    ssf_mash1_button(context, BUTTON_B, duration);
}
void spin_and_mash_A(ProControllerContext& context, Milliseconds duration){
    for (Milliseconds c = 0ms; c < duration; c += 1280ms){
        ssf_press_left_joystick(context, {0, -1}, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, {+1, 0}, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, {0, +1}, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);

        ssf_press_left_joystick(context, {-1, 0}, 0ms, 320ms);
        ssf_press_button(context, BUTTON_A, 160ms);
        ssf_press_button(context, BUTTON_A, 160ms);
    }
}
void travel_to_spin_location(ProControllerContext& context){
    ssf_press_left_joystick(context, {+1, -0.126}, 800ms, 2000ms);
    ssf_press_button(context, BUTTON_A, 800ms);
    ssf_press_button(context, BUTTON_A, 400ms);
    ssf_press_left_joystick(context, {+1, -1}, 400ms, 400ms);
}
void travel_back_to_lady(ProControllerContext& context){
    ssf_press_left_joystick(context, {0, -1}, 240ms, 240ms);
    ssf_press_left_joystick(context, {+1, -0.126}, 2080ms, 2080ms);
    ssf_press_left_joystick(context, {-1, 0}, 400ms, 3200ms);
    ssf_mash1_button(context, BUTTON_B, 800ms);
    ssf_do_nothing(context, 2400ms);
    ssf_press_left_joystick(context, {+0.5, +1}, 960ms, 960ms);
    ssf_press_left_joystick(context, {+1, +1}, 960ms, 960ms);
}



}
}

