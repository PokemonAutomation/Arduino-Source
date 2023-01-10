// EGG FETCHER SCVI

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"
#include "EggFetcher.h"

int count = 0;
int rounds = 0;

int main(void){
// START PROGRAM ON MICROCONTROLLER
start_program_callback();
initialize_framework(0);

// START PROGRAM ON SWITCH
start_program_flash(CONNECT_CONTROLLER_DELAY);
grip_menu_connect_go_home();

// START GAME (BE SURE TO HAVE INSTRUCTIONS SET UP)
pbf_press_button(BUTTON_HOME, 10, 375);

while (rounds != MAX_SAND){
    // DELAY START
    pbf_wait(50);
    
    // MAKE SANDWICH
    pbf_move_left_joystick(0x80, 0xff, 250, 105);
    pbf_press_button(BUTTON_A, 5, 250);
    pbf_press_button(BUTTON_A, 5, 750);
    pbf_press_button(BUTTON_X, 5, 105);
    if (USE_HM == true) {
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 250);
        while (count != BUTTER_POS) {
            pbf_press_dpad(DPAD_DOWN, 5, 105);
            count++;
        };
        count = 0;
        pbf_press_dpad(DPAD_UP, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        while (count != BUTTER_POS) {
            pbf_press_dpad(DPAD_UP, 5, 105);
            count++;
        };
        while (count != HERBA_POS) {
            pbf_press_dpad(DPAD_UP, 5, 105);
            count++;
        };
        count = 0;
        pbf_press_dpad(DPAD_DOWN, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 250);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 1250);
        
        pbf_move_left_joystick(0x80, 0xff, 250, 105);
        pbf_move_left_joystick(0x80, 0x00, 65, 105);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 500);
        pbf_press_button(BUTTON_A, 5, 2500);
        pbf_press_button(BUTTON_A, 5, 105);
        
    };
    if (USE_HM == false) {
        while (count != STB_POS) {
            pbf_press_dpad(DPAD_UP, 5, 105);
            count++;
        };
        count=0;
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 250);
        while (count != BUTTER_POS) {
            pbf_press_dpad(DPAD_DOWN, 5, 105);
            count++;
        };
        count=0;
        pbf_press_dpad(DPAD_UP, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 250);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_PLUS, 5, 1250);
        
        pbf_move_left_joystick(0x80, 0xff, 250, 105);
        pbf_move_left_joystick(0x80, 0x00, 65, 105);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 500);
        pbf_press_button(BUTTON_A, 5, 2500);
        pbf_press_button(BUTTON_A, 5, 105);
    };
    pbf_move_left_joystick(0x80, 0x00, 5, 105);
    while (count != 10) {
        pbf_wait(22500);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
        pbf_press_button(BUTTON_A, 5, 105);
    };
    rounds++;
}
    //EXIT TO IDLE MODE (NEVER USED)
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};

