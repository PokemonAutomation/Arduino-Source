/*  Mass Release Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ReleaseHelpers_H
#define PokemonAutomation_PokemonSwSh_ReleaseHelpers_H

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_BoxHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



// Release one pokemon in box
static void release(ProControllerContext& context){
    ssf_press_button(context, BUTTON_A, 600ms, 80ms);
    ssf_press_dpad_ptv(context, DPAD_UP, 160ms, 80ms);
    ssf_press_dpad_ptv(context, DPAD_UP, 160ms, 80ms);
    ssf_press_button(context, BUTTON_A, 1000ms, 80ms);
    ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms, 80ms);
//    ssf_press_button(context, BUTTON_A, 150);
//    ssf_press_button(context, BUTTON_A, 40);
    ssf_mash_AZs(context, 230);
}
static void release_box(ProControllerContext& context){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            box_scroll(context, DPAD_DOWN);
            box_scroll(context, DPAD_RIGHT);
            box_scroll(context, DPAD_RIGHT);
        }
        for (uint8_t col = 0; col < 6; col++){
            if (col != 0){
                box_scroll(context, DPAD_RIGHT);
            }
            release(context);
        }
    }
}
static void release_boxes(ProControllerContext& context, uint8_t boxes){
    if (boxes == 0){
        return;
    }
    release_box(context);

    Milliseconds box_change_delay = GameSettings::instance().BOX_CHANGE_DELAY0;
    for (uint8_t box = 1; box < boxes; box++){
        box_scroll(context, DPAD_DOWN);
        box_scroll(context, DPAD_DOWN);
        box_scroll(context, DPAD_DOWN);
        box_scroll(context, DPAD_RIGHT);
        box_scroll(context, DPAD_RIGHT);
        ssf_press_button_ptv(context, BUTTON_R, box_change_delay, 104ms);
        release_box(context);
    }
}



}
}
}
#endif

