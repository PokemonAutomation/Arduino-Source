// EGG FETCHER SCVI

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"
#include "EggFetcher.h"

void fly_to_gate(void) {
    pbf_press_button(BUTTON_A, 1, 500);
    pbf_press_button(BUTTON_Y, 1, 800);
    pbf_move_left_joystick(STICK_MAX, STICK_MAX, 6, 500);
    pbf_mash_button(BUTTON_A, 1000);
};

void open_picnic(void) {
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 190, 0);
    pbf_press_button(BUTTON_L, 5, 100);
    pbf_press_button(BUTTON_X, 1, 200);
    pbf_press_button(BUTTON_A, 1, 1000);
}

void move_to_basket(void) {
    pbf_press_button(BUTTON_L, 5, 100);
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 50, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 70, 0);
    pbf_press_button(BUTTON_L, 5, 100);
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 50, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 100, 50);
    pbf_press_button(BUTTON_L, 5, 100);
    pbf_press_button(BUTTON_PLUS, 5, 200);
}

int main(void){
    // START PROGRAM ON MICROCONTROLLER
    start_program_callback();
    initialize_framework(0);

    // START IN GRIP MENU
    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    if (TOLERATE_SYSTEM_UPDATE_MENU_FAST) {
        pbf_press_button(BUTTON_A, 5, 180);
        pbf_move_right_joystick(STICK_CENTER, STICK_MIN, 5, 0);
    };
    pbf_press_button(BUTTON_A, 5, 500);

    open_picnic();
    move_to_basket();

    for (int curSand = 0; curSand < MAX_SAND; curSand++) {
        pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 250, 100);
        // START SANDWICH
        pbf_press_button(BUTTON_A, 5, 250);
        pbf_press_button(BUTTON_A, 5, 750);
        // SELECT RECIPE
        if (USE_HM) {
            pbf_press_dpad(DPAD_DOWN, 5, 100);
        }
        pbf_press_button(BUTTON_A, 5, 250);
        // SELECT PICK
        pbf_press_button(BUTTON_A, 5, 1250);
        // MAKE SANDWICH
        pbf_move_left_joystick(0x80, 0x00, 250, 100);
        pbf_move_left_joystick(0x80, 0xff, 25, 100);
        pbf_mash_button(BUTTON_A, 1500);
        pbf_wait(3000);
        pbf_press_button(BUTTON_A, 5, 150);
        // WAIT AT BASKET
        pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 5, 100);
        for (int rounds = 0; rounds < 10; rounds++) {
            pbf_wait(22500);
            pbf_press_button(BUTTON_A, 5, 150);
            pbf_mash_button(BUTTON_B, 4000);
        };
        pbf_wait(200);
    };
    // EXIT TO IDLE MODE
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};