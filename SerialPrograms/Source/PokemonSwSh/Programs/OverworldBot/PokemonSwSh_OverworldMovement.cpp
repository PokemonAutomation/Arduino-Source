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
        pbf_move_left_joystick(context, 255, 128, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 128ms, 0ms);
    }else{
        pbf_move_left_joystick(context, {-1, 0}, 128ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 128, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 128ms, 0ms);
    }
}
void move_in_circle_down(ProControllerContext& context, bool counter_clockwise){
    if (counter_clockwise){
        pbf_move_left_joystick(context, {-1, 0}, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 128, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 192ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 192ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 192ms, 0ms);
    }else{
        pbf_move_left_joystick(context, 255, 128, 128ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, 128, 255, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 128ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 128ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 192ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 192ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 192ms, 0ms);
    }
}
void circle_in_place(ProControllerContext& context, bool counter_clockwise){
    if (counter_clockwise){
        pbf_move_left_joystick(context, {-1, 0}, 512ms, 0ms);    //  Correct for bias.
        pbf_move_left_joystick(context, 128, 255, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 128, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 256ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 128, 128ms, 0ms);  //  Correct for bias.
    }else{
        pbf_move_left_joystick(context, 255, 128, 512ms, 0ms);  //  Correct for bias.
        pbf_move_left_joystick(context, 128, 255, 256ms, 0ms);
        pbf_move_left_joystick(context, {-1, -1}, 256ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 256ms, 0ms);
        pbf_move_left_joystick(context, 0, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 0, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 128, 256ms, 0ms);
        pbf_move_left_joystick(context, 255, 255, 256ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 128ms, 0ms);    //  Correct for bias.
    }
}
void move_in_line(ProControllerContext& context, bool horizontal){
    if (horizontal){
        pbf_move_left_joystick(context, {-1, 0}, 1024ms, 256ms);
        pbf_move_left_joystick(context, 255, 128, 1024ms, 256ms);
    }else{
        pbf_move_left_joystick(context, 128, 255, 1024ms, 256ms);
        pbf_move_left_joystick(context, 128, 0, 1024ms, 256ms);
    }
}


}
}
}

