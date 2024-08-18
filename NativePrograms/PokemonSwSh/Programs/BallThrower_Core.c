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
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"
#include "NativePrograms/PokemonSwSh/Programs/BallThrower.h"


int main(void){
    start_program_callback();
    initialize_framework(0);

    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    pbf_press_button(BUTTON_HOME, 10, PokemonSwSh_HOME_TO_GAME_DELAY);

    while (true){
        pbf_press_button(BUTTON_X, 50, 50);
        pbf_press_button(BUTTON_A, 50, 50);
        pbf_mash_button(BUTTON_B, 100);
    }

    pbf_press_button(BUTTON_HOME, 10, PokemonSwSh_GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
};

