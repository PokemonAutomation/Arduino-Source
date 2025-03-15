// MASS PURCHASE

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSV/Programs/MassPurchase.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"

int main(void){
// START PROGRAM ON MICROCONTROLLER
start_program_callback();
initialize_framework(0);
int finish=0;
int count=0;

// START PROGRAM ON SWITCH
start_program_flash(CONNECT_CONTROLLER_DELAY);
grip_menu_connect_go_home();

// START GAME (BE SURE TO HAVE INSTRUCTIONS SET UP)
pbf_press_button(BUTTON_HOME, 10, 375);

// BUY
while (finish != ITEMS) {
    pbf_press_button(BUTTON_A, 5, 105);
    while (count != QUANTITY) {
        pbf_press_dpad(DPAD_UP, 5, 105);
        count++;
    };
    pbf_press_dpad(DPAD_DOWN, 5, 105);
    pbf_press_button(BUTTON_A, 5, 125);
    if (PAY_LP == true) {
        pbf_press_dpad(DPAD_DOWN, 5, 105);
    };
    pbf_press_button(BUTTON_A, 5, 250);
    pbf_press_button(BUTTON_A, 5, 125);
    pbf_press_dpad(DPAD_DOWN, 5, 105);
};
    pbf_press_button(BUTTON_B, 5, 105);
    pbf_press_button(BUTTON_A, 5, 105);
    pbf_press_button(BUTTON_B, 5, 105);
    pbf_press_button(BUTTON_A, 5, 105);
    
    // EXIT TO IDLE MODE AFTER FINISHED
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};

