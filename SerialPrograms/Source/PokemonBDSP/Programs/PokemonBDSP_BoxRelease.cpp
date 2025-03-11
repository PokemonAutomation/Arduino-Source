/*  Box Release Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_BoxRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

void detach(ProControllerContext& context){
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    pbf_move_right_joystick(context, 128, 255, 20, 10);
    pbf_press_button(context, BUTTON_ZL, 20, 85);
    pbf_press_button(context, BUTTON_ZL, 20, 85);
//    pbf_move_right_joystick(context, 128, 255, 20, 0);
    pbf_press_button(context, BUTTON_B, 20, 85);
}
void detach_box(ProControllerContext& context, uint16_t box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            pbf_move_right_joystick(context, 128, 255, 20, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 20, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 20, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
//            context->wait_for_all_requests();
            if (col != 0){
                pbf_move_right_joystick(context, 255, 128, 20, box_scroll_delay);
            }
//            context->wait_for_all_requests();
            detach(context);
        }
    }
}





void release(ProControllerContext& context){
    pbf_press_button(context, BUTTON_ZL, 20, 50);
    pbf_move_right_joystick(context, 128, 0, 20, 10);
    pbf_move_right_joystick(context, 128, 0, 20, 10);
    pbf_press_button(context, BUTTON_ZL, 20, 105);
    pbf_move_right_joystick(context, 128, 255, 20, 10);
    pbf_mash_button(context, BUTTON_ZL, 120);
    pbf_wait(context, 30);
}
void release_box(ProControllerContext& context, Milliseconds box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            pbf_move_right_joystick(context, 128, 255, 160ms, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
//            context->wait_for_all_requests();
            if (col != 0){
                pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
            }
//            context->wait_for_all_requests();
            release(context);
        }
    }
}
void release_boxes(
    ProControllerContext& context,
    uint8_t boxes,
    Milliseconds box_scroll_delay,
    Milliseconds box_change_delay
){
    if (boxes == 0){
        return;
    }
    release_box(context, box_scroll_delay);
    for (uint8_t box = 1; box < boxes; box++){
        pbf_move_right_joystick(context, 128, 255, 160ms, box_scroll_delay);
        pbf_move_right_joystick(context, 128, 255, 160ms, box_scroll_delay);
        pbf_move_right_joystick(context, 128, 255, 160ms, box_scroll_delay);
        pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
        pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
        pbf_wait(context, 50);
        pbf_press_button(context, BUTTON_R, 160ms, box_change_delay);
        release_box(context, box_scroll_delay);
    }
}


}
}
}
