/*  Mass Release Helpers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ReleaseHelpers_H
#define PokemonAutomation_PokemonSwSh_ReleaseHelpers_H

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

// Release one pokemon in box
static void release(SwitchControllerContext& context){
    ssf_press_button(context, BUTTON_A, 60, 10);
    ssf_press_dpad(context, DPAD_UP, 15);
    ssf_press_dpad(context, DPAD_UP, 15);
    ssf_press_button(context, BUTTON_A, 125, 10);
    ssf_press_dpad(context, DPAD_DOWN, 10);
//    ssf_press_button(context, BUTTON_A, 150);
//    ssf_press_button(context, BUTTON_A, 40);
    ssf_mash_AZs(context, 180);
}
static void release_box(SwitchControllerContext& context, Milliseconds box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            ssf_press_dpad(context, DPAD_DOWN, box_scroll_delay);
            ssf_press_dpad(context, DPAD_RIGHT, box_scroll_delay);
            ssf_press_dpad(context, DPAD_RIGHT, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
            if (col != 0){
                ssf_press_dpad(context, DPAD_RIGHT, box_scroll_delay);
            }
            release(context);
        }
    }
}
static void release_boxes(
    SwitchControllerContext& context,
    uint8_t boxes,
    Milliseconds box_scroll_delay,
    Milliseconds box_change_delay
){
    if (boxes == 0){
        return;
    }
    release_box(context, box_scroll_delay);
    for (uint8_t box = 1; box < boxes; box++){
        ssf_press_dpad(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad(context, DPAD_RIGHT, box_scroll_delay);
        ssf_press_dpad(context, DPAD_RIGHT, box_scroll_delay);
        ssf_press_button(context, BUTTON_R, box_change_delay);
        release_box(context, box_scroll_delay);
    }
}



}
}
}
#endif

