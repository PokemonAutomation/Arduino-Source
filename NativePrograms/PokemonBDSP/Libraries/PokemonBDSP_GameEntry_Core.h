/*  Game Entry Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonBDSP_GameEntry_Core_H
#define PokemonAutomation_PokemonBDSP_GameEntry_Core_H

#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_GameEntry.h"
#include "NativePrograms/PokemonBDSP/Libraries/PokemonBDSP_Settings.h"

void PokemonBDSP_resume_game_no_interact(bool tolerate_update_menu){
    if (tolerate_update_menu){
        pbf_press_button(BUTTON_HOME, 10, PokemonBDSP_HOME_TO_GAME_DELAY);
        pbf_move_right_joystick(128, 255, 10, 10);
        pbf_move_right_joystick(128, 0, 10, 10);
        pbf_press_button(BUTTON_A, 10, PokemonBDSP_HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(BUTTON_HOME, 10, PokemonBDSP_HOME_TO_GAME_DELAY);
    }
}


void PokemonBDSP_openedgame_to_gamemenu(uint16_t timeout){
    pbf_wait(timeout);
}
void PokemonBDSP_gamemenu_to_ingame(
    uint16_t mash_duration, uint16_t enter_game_timeout
){
    pbf_mash_button(BUTTON_A, mash_duration);
    pbf_wait(enter_game_timeout);
}
void PokemonBDSP_openedgame_to_ingame(
    uint16_t load_game_timeout,
    uint16_t mash_duration, uint16_t enter_game_timeout,
    uint16_t post_wait_time
){
    PokemonBDSP_openedgame_to_gamemenu(load_game_timeout);
    PokemonBDSP_gamemenu_to_ingame(mash_duration, enter_game_timeout);
    pbf_wait(post_wait_time);
}

void PokemonBDSP_move_to_user(uint8_t user_slot){
    if (user_slot != 0){
        //  Move to correct user.
        for (uint8_t c = 0; c < 8; c++){
            pbf_press_dpad(DPAD_LEFT, 7, 7);
        }
//        pbf_wait(50);
        for (uint8_t c = 1; c < user_slot; c++){
            pbf_press_dpad(DPAD_RIGHT, 7, 7);
        }
    }
}

void PokemonBDSP_start_game_from_home(
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_mash
){
    if (game_slot != 0){
        pbf_press_button(BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(BUTTON_A, 5, 175);     //  Choose game
        pbf_press_dpad(DPAD_UP, 5, 0);          //  Skip the update window.
        PokemonBDSP_move_to_user(user_slot);
    }

//    cout << "START_GAME_REQUIRES_INTERNET = " << START_GAME_REQUIRES_INTERNET << endl;
    if (!START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(BUTTON_A, start_game_mash);
    }else{
        pbf_press_button(BUTTON_A, 5, 175);     //  Enter select user menu.
        PokemonBDSP_move_to_user(user_slot);
        pbf_press_button(BUTTON_A, 5, 5);       //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = start_game_mash;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(BUTTON_ZR, duration);
    }
}



void PokemonBDSP_reset_game_from_home(
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    if (START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game();
        PokemonBDSP_start_game_from_home(
            tolerate_update_menu,
            0, 0,
            PokemonBDSP_START_GAME_MASH
        );
    }else{
        pbf_press_button(BUTTON_X, 50, 0);
        pbf_mash_button(BUTTON_A, PokemonBDSP_START_GAME_MASH);
    }
    PokemonBDSP_openedgame_to_ingame(
        PokemonBDSP_START_GAME_WAIT,
        PokemonBDSP_ENTER_GAME_MASH,
        PokemonBDSP_ENTER_GAME_WAIT,
        post_wait_time
    );
}

void PokemonBDSP_save_game(void){
    pbf_press_button(BUTTON_X, 10, PokemonBDSP_OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(BUTTON_R, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(BUTTON_A, 10, 5 * TICKS_PER_SECOND);
}


#endif

