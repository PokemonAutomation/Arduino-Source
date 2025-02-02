/*  EggHelpers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggHelpers_H
#define PokemonAutomation_PokemonSwSh_EggHelpers_H

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

#define GO_TO_LADY_DURATION         51
#define TRAVEL_RIGHT_DURATION       300
#define END_BATCH_MASH_B_DURATION   (20 * TICKS_PER_SECOND)


//  Collect egg.
static void collect_egg(SwitchControllerContext& context){
    ssf_press_button1(context, BUTTON_A, 120);
    if (GameSettings::instance().EGG_FETCH_EXTRA_LINE){
        ssf_press_button1(context, BUTTON_A, 120);
    }
    ssf_press_button1(context, BUTTON_A, 10);
}
static void collect_egg_mash_out(SwitchControllerContext& context, bool deposit_automatically){
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
//  Assume the selected app in the menu is Twon Map.
static void fly_home(SwitchControllerContext& context, char from_overworld){
    if (from_overworld){
        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    }
    ssf_press_button2(context, BUTTON_A, 400, 10);
    ssf_press_dpad2(context, DPAD_UP_RIGHT, 25, 5);
    pbf_mash_button(context, BUTTON_A, 480);
}

//  Assume the selected app in the menu is Twon Map.
static void fly_home_goto_lady(SwitchControllerContext& context, char from_overworld){
    fly_home(context, from_overworld);

    //  Go to lady.
    //  If you change this, you MUST update "GO_TO_LADY_DURATION".
    ssf_press_joystick2(context, true, STICK_MIN, STICK_CENTER, 16, 6);
    ssf_press_joystick2(context, true, STICK_CENTER, STICK_MIN, 90, 45);
}

//  Assume the selected app in the menu is Twon Map.
static void fly_home_collect_egg(SwitchControllerContext& context, char from_overworld){
    fly_home_goto_lady(context, from_overworld);
    collect_egg(context);
}



//  EggHatcher+EggCombined Helpers

//#define EGG_BUTTON_HOLD_DELAY   10
static const Milliseconds EGG_BUTTON_HOLD_DELAY = 80ms;

// - From game menu to pokemon storage box
// - Move cursor to the second pokemon in the party, aka first hatched pokemon in the party
// - Press button Y two times to change pokemon selection to group selection
static void menu_to_box(SwitchControllerContext& context, bool from_map){
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    if (from_map){
        ssf_press_dpad(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    }
    ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, EGG_BUTTON_HOLD_DELAY);
    ssf_press_dpad(context, DPAD_LEFT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    ssf_press_dpad(context, DPAD_DOWN, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button(context, BUTTON_Y, 240ms, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button(context, BUTTON_Y, 240ms, EGG_BUTTON_HOLD_DELAY);
}
static void box_to_menu(SwitchControllerContext& context){
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

    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    ssf_press_button(context, BUTTON_B, 160ms, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button(context, BUTTON_B, GameSettings::instance().BOX_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);

    //  Back out to menu.
    ssf_press_button(context, BUTTON_B, GameSettings::instance().POKEMON_TO_MENU_DELAY0, EGG_BUTTON_HOLD_DELAY);

    ssf_press_dpad(context, DPAD_LEFT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    ssf_press_dpad(context, DPAD_DOWN, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
}

static void party_to_column(SwitchControllerContext& context, uint8_t column){
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    ssf_press_dpad(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    column++;
    if (column <= 3){
        for (uint8_t c = 0; c != column; c++){
            ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }
    }else{
        for (uint8_t c = 7; c != column; c--){
            ssf_press_dpad(context, DPAD_LEFT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }
    }
}
static void column_to_party(SwitchControllerContext& context, uint8_t column){
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    column++;
    if (column <= 3){
        for (uint8_t c = column; c != 0; c--){
            ssf_press_dpad(context, DPAD_LEFT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }
    }else{
        for (uint8_t c = column; c != 7; c++){
            ssf_press_dpad(context, DPAD_RIGHT, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
        }
    }
    ssf_press_dpad(context, DPAD_DOWN, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
}

static void pickup_column(SwitchControllerContext& context, char party){
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    ssf_press_button(context, BUTTON_A, 160ms, EGG_BUTTON_HOLD_DELAY);
    if (party){
        ssf_press_dpad(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    }
    ssf_press_dpad(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
    ssf_press_button(context, BUTTON_A, GameSettings::instance().BOX_PICKUP_DROP_DELAY0, EGG_BUTTON_HOLD_DELAY);
}




}
}
}
#endif
