/*  EggHelpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggHelpers_H
#define PokemonAutomation_PokemonSwSh_EggHelpers_H

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

#define GO_TO_LADY_DURATION         (51 * 8ms)
#define TRAVEL_RIGHT_DURATION       (300 * 8ms)
#define END_BATCH_MASH_B_DURATION   (20 * TICKS_PER_SECOND * 8ms)


//  Collect egg.
static void collect_egg(ProControllerContext& context){
    ssf_press_button(context, BUTTON_A, 960ms);
    if (GameSettings::instance().EGG_FETCH_EXTRA_LINE){
        ssf_press_button(context, BUTTON_A, 960ms);
    }
    ssf_press_button(context, BUTTON_A, 80ms, 80ms);
}
static void collect_egg_mash_out(ProControllerContext& context, bool deposit_automatically){
    Milliseconds FETCH_EGG_MASH_DELAY = GameSettings::instance().FETCH_EGG_MASH_DELAY0;
    pbf_mash_button(
        context,
        BUTTON_B,
        deposit_automatically
            ? FETCH_EGG_MASH_DELAY
            : FETCH_EGG_MASH_DELAY + 1920ms
    );
}


//  Fly Home: Used by everything.
//  Assume the selected app in the menu is Town Map.
static void fly_home(ProControllerContext& context, char from_overworld){
    if (from_overworld){
        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    }
    ssf_press_button(context, BUTTON_A, 3200ms, 160ms);
    ssf_press_right_joystick(context, 160, 96, 160ms, 160ms);
    pbf_mash_button(context, BUTTON_A, 480);
}

//  Assume the selected app in the menu is Twon Map.
static void fly_home_goto_lady(ProControllerContext& context, char from_overworld){
    fly_home(context, from_overworld);

    //  Go to lady.
    //  If you change this, you MUST update "GO_TO_LADY_DURATION".
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 20, 10);
    ssf_press_left_joystick(context, STICK_CENTER, STICK_MIN, 90, 45);
}

//  Assume the selected app in the menu is Twon Map.
static void fly_home_collect_egg(ProControllerContext& context, char from_overworld){
    fly_home_goto_lady(context, from_overworld);
    collect_egg(context);
}



//  EggHatcher+EggCombined Helpers

//#define EGG_BUTTON_HOLD_DELAY   10
static const Milliseconds EGG_BUTTON_HOLD_DELAY = 80ms;

// - From game menu to pokemon storage box
// - Move cursor to the second pokemon in the party, aka first hatched pokemon in the party
// - Press button Y two times to change pokemon selection to group selection
static void menu_to_box(ProControllerContext& context, bool from_map){
    if (from_map){
        box_scroll(context, DPAD_UP);
        box_scroll(context, DPAD_RIGHT);
    }
    ssf_press_button_ptv(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button_ptv(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, EGG_BUTTON_HOLD_DELAY);
    box_scroll(context, DPAD_LEFT);
    box_scroll(context, DPAD_DOWN);
    ssf_press_button_ptv(context, BUTTON_Y, 240ms, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button_ptv(context, BUTTON_Y, 240ms, EGG_BUTTON_HOLD_DELAY);
}
static void box_to_menu(ProControllerContext& context){
    //  There are two states here which need to be merged:
    //      1.  The depositing column was empty. The party has been swapped and
    //          it's sitting in the box with no held pokemon.
    //      2.  The depositing column was not empty. The party swap failed and
    //          it's sitting in the box holding on the party pokemon.
    //
    //  Double press B quickly here to back out of the box.
    //  In state (1):   The 1st B will begin back out of the box. The 2nd B will
    //                  be swallowed by the animation.
    //  In state (2):   The 1st B will drop the party pokemon. The 2nd B will
    //                  back out of the box.

    ssf_press_button_ptv(context, BUTTON_B, 160ms, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button_ptv(context, BUTTON_B, GameSettings::instance().BOX_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);

    //  Back out to menu.
    ssf_press_button_ptv(context, BUTTON_B, GameSettings::instance().POKEMON_TO_MENU_DELAY0, EGG_BUTTON_HOLD_DELAY);

    box_scroll(context, DPAD_LEFT);
    box_scroll(context, DPAD_DOWN);
}

static void party_to_column(ProControllerContext& context, uint8_t column){
    box_scroll(context, DPAD_UP);
    column++;
    if (column <= 3){
        for (uint8_t c = 0; c != column; c++){
            box_scroll(context, DPAD_RIGHT);
        }
    }else{
        for (uint8_t c = 7; c != column; c--){
            box_scroll(context, DPAD_LEFT);
        }
    }
}
static void column_to_party(ProControllerContext& context, uint8_t column){
    column++;
    if (column <= 3){
        for (uint8_t c = column; c != 0; c--){
            box_scroll(context, DPAD_LEFT);
        }
    }else{
        for (uint8_t c = column; c != 7; c++){
            box_scroll(context, DPAD_RIGHT);
        }
    }
    box_scroll(context, DPAD_DOWN);
}

static void pickup_column(ProControllerContext& context, char party){
    ssf_press_button_ptv(context, BUTTON_A, 160ms, EGG_BUTTON_HOLD_DELAY);
    if (party){
        box_scroll(context, DPAD_UP);
    }
    box_scroll(context, DPAD_UP);
    ssf_press_button_ptv(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY0, EGG_BUTTON_HOLD_DELAY);
}




}
}
}
#endif
