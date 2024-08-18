/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "NintendoSwitch_PushButtons.h"

void close_game(void){
    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.
    pbf_mash_button(BUTTON_X, 100);             //  Close game
    pbf_mash2_button(BUTTON_X, BUTTON_A, 50);   //  Confirm close game
    pbf_mash_button(BUTTON_X, 50);
    pbf_mash_button(BUTTON_B, 350);
}
void resume_game_no_interact(uint16_t delay, bool tolerate_update_menu){
    if (tolerate_update_menu){
        pbf_press_button(BUTTON_HOME, 10, delay);
        pbf_press_dpad(DPAD_DOWN, 10, 10);
        pbf_press_dpad(DPAD_UP, 10, 10);
        pbf_press_button(BUTTON_A, 10, delay);
    }else{
        pbf_press_button(BUTTON_HOME, 10, delay);
    }
}
void resume_game_back_out(uint16_t delay, bool tolerate_update_menu, uint16_t mash_B_time){
    if (tolerate_update_menu){
        pbf_press_button(BUTTON_HOME, 10, delay);
        pbf_press_dpad(DPAD_DOWN, 10, 10);
        pbf_press_dpad(DPAD_UP, 10, 10);
        pbf_press_button(BUTTON_A, 10, delay);
        pbf_mash_button(BUTTON_B, mash_B_time);
    }else{
        pbf_press_button(BUTTON_HOME, 10, delay);
    }
}

#endif
