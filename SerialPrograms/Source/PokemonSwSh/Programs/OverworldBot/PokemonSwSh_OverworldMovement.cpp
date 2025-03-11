/*  Overworld Movement
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_OverworldMovement.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_in_circle_up(ProControllerContext& context, bool counter_clockwise){
//    cout << "up" << endl;
    if (counter_clockwise){
        pbf_move_left_joystick(context, 255, 128, 16, 0);
        pbf_move_left_joystick(context, 255, 0, 16, 0);
        pbf_move_left_joystick(context, 128, 0, 16, 0);
        pbf_move_left_joystick(context, 0, 0, 16, 0);
        pbf_move_left_joystick(context, 0, 128, 16, 0);
        pbf_move_left_joystick(context, 0, 255, 16, 0);
        pbf_move_left_joystick(context, 128, 255, 16, 0);
        pbf_move_left_joystick(context, 255, 255, 16, 0);
    }else{
        pbf_move_left_joystick(context, 0, 128, 16, 0);
        pbf_move_left_joystick(context, 0, 0, 16, 0);
        pbf_move_left_joystick(context, 128, 0, 16, 0);
        pbf_move_left_joystick(context, 255, 0, 16, 0);
        pbf_move_left_joystick(context, 255, 128, 16, 0);
        pbf_move_left_joystick(context, 255, 255, 16, 0);
        pbf_move_left_joystick(context, 128, 255, 16, 0);
        pbf_move_left_joystick(context, 0, 255, 16, 0);
    }
}
void move_in_circle_down(ProControllerContext& context, bool counter_clockwise){
    if (counter_clockwise){
        pbf_move_left_joystick(context, 0, 128, 16, 0);
        pbf_move_left_joystick(context, 0, 255, 16, 0);
        pbf_move_left_joystick(context, 128, 255, 16, 0);
        pbf_move_left_joystick(context, 255, 255, 16, 0);
        pbf_move_left_joystick(context, 255, 128, 16, 0);
        pbf_move_left_joystick(context, 255, 0, 24, 0);
        pbf_move_left_joystick(context, 128, 0, 24, 0);
        pbf_move_left_joystick(context, 0, 0, 24, 0);
    }else{
        pbf_move_left_joystick(context, 255, 128, 16, 0);
        pbf_move_left_joystick(context, 255, 255, 16, 0);
        pbf_move_left_joystick(context, 128, 255, 16, 0);
        pbf_move_left_joystick(context, 0, 255, 16, 0);
        pbf_move_left_joystick(context, 0, 128, 16, 0);
        pbf_move_left_joystick(context, 0, 0, 24, 0);
        pbf_move_left_joystick(context, 128, 0, 24, 0);
        pbf_move_left_joystick(context, 255, 0, 24, 0);
    }
}
void circle_in_place(ProControllerContext& context, bool counter_clockwise){
    if (counter_clockwise){
        pbf_move_left_joystick(context, 0, 128, 64, 0);     //  Correct for bias.
        pbf_move_left_joystick(context, 128, 255, 32, 0);
        pbf_move_left_joystick(context, 255, 255, 32, 0);
        pbf_move_left_joystick(context, 255, 128, 32, 0);
        pbf_move_left_joystick(context, 255, 0, 32, 0);
        pbf_move_left_joystick(context, 128, 0, 32, 0);
        pbf_move_left_joystick(context, 0, 0, 32, 0);
        pbf_move_left_joystick(context, 0, 128, 32, 0);
        pbf_move_left_joystick(context, 0, 255, 32, 0);
        pbf_move_left_joystick(context, 255, 128, 16, 0);   //  Correct for bias.
    }else{
        pbf_move_left_joystick(context, 255, 128, 64, 0);   //  Correct for bias.
        pbf_move_left_joystick(context, 128, 255, 32, 0);
        pbf_move_left_joystick(context, 0, 255, 32, 0);
        pbf_move_left_joystick(context, 0, 128, 32, 0);
        pbf_move_left_joystick(context, 0, 0, 32, 0);
        pbf_move_left_joystick(context, 128, 0, 32, 0);
        pbf_move_left_joystick(context, 255, 0, 32, 0);
        pbf_move_left_joystick(context, 255, 128, 32, 0);
        pbf_move_left_joystick(context, 255, 255, 32, 0);
        pbf_move_left_joystick(context, 0, 128, 16, 0);     //  Correct for bias.
    }
}
void move_in_line(ProControllerContext& context, bool horizontal){
    if (horizontal){
        pbf_move_left_joystick(context, 0, 128, 128, 32);
        pbf_move_left_joystick(context, 255, 128, 128, 32);
    }else{
        pbf_move_left_joystick(context, 128, 255, 128, 32);
        pbf_move_left_joystick(context, 128, 0, 128, 32);
    }
}


}
}
}

