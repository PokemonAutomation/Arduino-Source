// SCVI MASS RELEASE

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSV/Programs/MassRelease.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"

int main(void){
// START PROGRAM ON MICROCONTROLLER
start_program_callback();
initialize_framework(0);

// START PROGRAM ON SWITCH
start_program_flash(CONNECT_CONTROLLER_DELAY);
grip_menu_connect_go_home();
int comp = 0;
int done = 0;
int box_count = 0;
    
// START GAME (BE SURE TO HAVE INSTRUCTIONS SET UP)
pbf_press_button(BUTTON_HOME, 10, 375);
    
while (done != RELEASE){
    // DELAY START
    pbf_wait(50);

    // RELEASE POKEMON
    pbf_press_button(BUTTON_A, 5, 125);
    pbf_press_dpad(DPAD_UP, 5, 105);
    pbf_press_dpad(DPAD_UP, 5, 105);
    pbf_press_button(BUTTON_A, 5, 125);
    pbf_press_dpad(DPAD_UP, 5, 105);
    pbf_press_button(BUTTON_A, 5, 500);
    pbf_press_button(BUTTON_A, 5, 105);
    comp++;
    box_count++;
    done++;
    
    // GO TO NEXT POKEMON
    if (box_count == 30) {
        pbf_press_button(BUTTON_R, 5, 105);
        pbf_press_dpad(DPAD_DOWN, 5, 105);
        pbf_press_dpad(DPAD_DOWN, 5, 105);
        box_count=0;
    };
    
    if (comp == 6) {
        pbf_press_dpad(DPAD_DOWN, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        pbf_press_dpad(DPAD_LEFT, 5, 105);
        comp=0;
    };
    
    pbf_press_dpad(DPAD_RIGHT, 5, 105);
    
    // FIX ERROR ONCE PER BOX
    if (box_count == 0) {
        pbf_press_button(BUTTON_B, 5, 250);
        pbf_press_button(BUTTON_B, 5, 250);
        pbf_press_button(BUTTON_B, 5, 250);
        pbf_press_button(BUTTON_B, 5, 250);
        pbf_press_button(BUTTON_X, 5, 105);
        pbf_press_dpad(DPAD_RIGHT, 5, 105);
        pbf_press_dpad(DPAD_UP, 500, 105);
        pbf_press_dpad(DPAD_DOWN, 5, 105);
        pbf_press_button(BUTTON_A, 5, 500);
    };
};
    
    // EXIT TO IDLE MODE WHEN DONE RELEASING
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};
