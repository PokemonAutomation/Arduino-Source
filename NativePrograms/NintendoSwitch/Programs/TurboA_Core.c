/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_GameEntry.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"
//#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_GameEntry_Core.h"
#include "NativePrograms/NintendoSwitch/Programs/TurboA.h"

int main(void){
    start_program_callback();
    initialize_framework(0);

    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    resume_game_no_interact(3 * TICKS_PER_SECOND, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    // the resume game sequence opens the camera in Scarlet/Violet
    // mashing B closes the camera
    pbf_mash_button(BUTTON_B, 100);
    while (true){
        pbf_press_button(BUTTON_A, 5, 5);
    }

    pbf_press_button(BUTTON_HOME, 10, PokemonSwSh_GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
};

