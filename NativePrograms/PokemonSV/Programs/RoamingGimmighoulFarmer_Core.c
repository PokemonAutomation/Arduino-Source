// COIN FARMER ROAMING

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSV/Programs/RoamingGimmighoulFarmer.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"

int main(void){
// START PROGRAM ON MICROCONTROLLER
start_program_callback();
initialize_framework(0);
int finish=0;

// START PROGRAM ON SWITCH
start_program_flash(CONNECT_CONTROLLER_DELAY);
grip_menu_connect_go_home();

// START GAME (BE SURE TO HAVE INSTRUCTIONS SET UP)
pbf_press_button(BUTTON_HOME, 10, 375);

// FARM
    while (finish != SKIPS) {
        // COLLECT COIN
        pbf_press_button(BUTTON_A, 5, 375);
        
        // SAVE GAME
        pbf_press_button(BUTTON_X, 5, 250);
        pbf_press_button(BUTTON_R, 5, 250);
        pbf_press_button(BUTTON_A, 5, 625);
        pbf_press_button(BUTTON_A, 5, 105);
        
        // DATE SKIP
        pbf_press_button(BUTTON_HOME, 10, 125);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_DOWN, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_button(BUTTON_A, 5, 175);
        pbf_press_dpad(DPAD_DOWN, 500, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_DOWN, 100, 75);
        pbf_press_button(BUTTON_A, 5, 125);
        pbf_press_dpad(DPAD_DOWN, 100, 75);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_UP, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_dpad(DPAD_RIGHT, 5, 75);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_HOME, 5, 500);
        
        // RESTART GAME
        pbf_press_button(BUTTON_X, 5, 125);
        pbf_press_button(BUTTON_A, 5, 500);
        pbf_press_button(BUTTON_A, 5, 125);
        pbf_press_button(BUTTON_A, 5, 2500);
        pbf_press_button(BUTTON_A, 5, 3750);
    };
    // EXIT TO IDLE MODE AFTER FINISHED
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};

