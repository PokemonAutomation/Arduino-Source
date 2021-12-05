/*  Box Release Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP_BoxRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void release(const BotBaseContext& context){
    pbf_press_button(context, BUTTON_A, 20, 50);
    pbf_move_right_joystick(context, 128, 0, 10, 10);
    pbf_move_right_joystick(context, 128, 0, 10, 10);
    pbf_press_button(context, BUTTON_A, 10, 115);
    pbf_move_right_joystick(context, 128, 255, 10, 10);
    pbf_mash_button(context, BUTTON_A, 120);
    pbf_wait(context, 30);
}
void release_box(const BotBaseContext& context, uint16_t box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            pbf_move_right_joystick(context, 128, 255, 10, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 10, box_scroll_delay);
            pbf_move_right_joystick(context, 255, 128, 10, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
//            context->wait_for_all_requests();
            if (col != 0){
                pbf_move_right_joystick(context, 255, 128, 10, box_scroll_delay);
            }
//            context->wait_for_all_requests();
            release(context);
        }
    }
}
void release_boxes(
    const BotBaseContext& context,
    uint8_t boxes,
    uint16_t box_scroll_delay,
    uint16_t box_change_delay
){
    if (boxes == 0){
        return;
    }
    release_box(context, box_scroll_delay);
    for (uint8_t box = 1; box < boxes; box++){
        pbf_press_dpad(context, DPAD_DOWN, 5, box_scroll_delay);
        pbf_press_dpad(context, DPAD_DOWN, 5, box_scroll_delay);
        pbf_press_dpad(context, DPAD_DOWN, 5, box_scroll_delay);
        pbf_press_dpad(context, DPAD_RIGHT, 5, box_scroll_delay);
        pbf_press_dpad(context, DPAD_RIGHT, 5, box_scroll_delay);
        pbf_wait(context, 50);
        pbf_press_button(context, BUTTON_R, 5, box_change_delay);
        release_box(context, box_scroll_delay);
    }
}


}
}
}
