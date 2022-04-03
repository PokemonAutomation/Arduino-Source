/*  Mass Release Helpers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ReleaseHelpers_H
#define PokemonAutomation_PokemonSwSh_ReleaseHelpers_H

#include "Common/Compiler.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

static void release(BotBaseContext& context){
    ssf_press_button2(context, BUTTON_A, 60, 10);
    ssf_press_dpad1(context, DPAD_UP, 15);
    ssf_press_dpad1(context, DPAD_UP, 15);
    ssf_press_button2(context, BUTTON_A, 125, 10);
    ssf_press_dpad1(context, DPAD_DOWN, 10);
//    ssf_press_button1(context, BUTTON_A, 150);
//    ssf_press_button1(context, BUTTON_A, 40);
    mash_A(context, 180);
}
static void release_box(BotBaseContext& context, uint16_t box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            ssf_press_dpad1(context, DPAD_DOWN, box_scroll_delay);
            ssf_press_dpad1(context, DPAD_RIGHT, box_scroll_delay);
            ssf_press_dpad1(context, DPAD_RIGHT, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
            if (col != 0){
                ssf_press_dpad1(context, DPAD_RIGHT, box_scroll_delay);
            }
            release(context);
        }
    }
}
static void release_boxes(
    BotBaseContext& context,
    uint8_t boxes,
    uint16_t box_scroll_delay,
    uint16_t box_change_delay
){
    if (boxes == 0){
        return;
    }
    release_box(context, box_scroll_delay);
    for (uint8_t box = 1; box < boxes; box++){
        ssf_press_dpad1(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad1(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad1(context, DPAD_DOWN, box_scroll_delay);
        ssf_press_dpad1(context, DPAD_RIGHT, box_scroll_delay);
        ssf_press_dpad1(context, DPAD_RIGHT, box_scroll_delay);
        ssf_press_button1(context, BUTTON_R, box_change_delay);
        release_box(context, box_scroll_delay);
    }
}



}
}
}
#endif

