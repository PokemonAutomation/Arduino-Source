/*  Pokemon BDSP: Mass Release
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonBDSP/Libraries/PokemonBDSP_GameEntry_Core.h"
#include "NativePrograms/PokemonBDSP/Programs/MassRelease.h"



void release(void){
    pbf_press_button(BUTTON_ZL, 20, 50);
    pbf_move_right_joystick(128, 0, 20, 10);
    pbf_move_right_joystick(128, 0, 20, 10);
    pbf_press_button(BUTTON_ZL, 20, 105);
    pbf_move_right_joystick(128, 255, 20, 10);
    pbf_mash_button(BUTTON_ZL, 120);
    pbf_wait(30);
}
void release_box(uint16_t box_scroll_delay){
    for (uint8_t row = 0; row < 5; row++){
        if (row != 0){
            pbf_press_dpad(DPAD_DOWN, 20, box_scroll_delay);
            pbf_press_dpad(DPAD_RIGHT, 20, box_scroll_delay);
            pbf_press_dpad(DPAD_RIGHT, 20, box_scroll_delay);
        }
        for (uint8_t col = 0; col < 6; col++){
            if (col != 0){
                pbf_press_dpad(DPAD_RIGHT, 20, box_scroll_delay);
            }
            release();
        }
    }
}
void release_boxes(
    uint8_t boxes,
    uint16_t box_scroll_delay,
    uint16_t box_change_delay
){
    if (boxes == 0){
        return;
    }
    release_box(box_scroll_delay);
    for (uint8_t box = 1; box < boxes; box++){
        pbf_press_dpad(DPAD_DOWN, 20, box_scroll_delay);
        pbf_press_dpad(DPAD_DOWN, 20, box_scroll_delay);
        pbf_press_dpad(DPAD_DOWN, 20, box_scroll_delay);
        pbf_press_dpad(DPAD_RIGHT, 20, box_scroll_delay);
        pbf_press_dpad(DPAD_RIGHT, 20, box_scroll_delay);
        pbf_press_button(BUTTON_R, 20, box_change_delay);
        release_box(box_scroll_delay);
    }
}



int main(void){
    start_program_callback();
    initialize_framework(0);

    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    PokemonBDSP_resume_game_no_interact(DODGE_SYSTEM_UPDATE_WINDOW);

    release_boxes(BOXES_TO_RELEASE, PokemonBDSP_BOX_SCROLL_DELAY, PokemonBDSP_BOX_CHANGE_DELAY);
    pbf_press_button(BUTTON_HOME, 20, PokemonBDSP_GAME_TO_HOME_DELAY);

    end_program_callback();
    end_program_loop();
};

