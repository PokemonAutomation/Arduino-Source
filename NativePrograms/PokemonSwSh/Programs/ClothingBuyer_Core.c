/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_GameEntry.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"
#include "NativePrograms/PokemonSwSh/Programs/ClothingBuyer.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Obfuscation Macros
#ifdef OBFUSCATE_SYMBOLS

#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(void){
    start_program_callback();
    initialize_framework(0);

    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    resume_game_no_interact(PokemonSwSh_HOME_TO_GAME_DELAY, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    while (true){
        pbf_press_button(BUTTON_A, 10, 90);
        if (CATEGORY_ROTATION){
            pbf_press_dpad(DPAD_RIGHT, 10, 40);
        }
        pbf_press_button(BUTTON_A, 10, 90);
        pbf_press_button(BUTTON_A, 10, 90);
        pbf_press_dpad(DPAD_DOWN, 10, 40);
    }
}

