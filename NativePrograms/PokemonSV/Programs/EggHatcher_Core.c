#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "EggHatcher.h"

void fly_to_gate(void) {
    pbf_press_button(BUTTON_A, 20, 500);
    pbf_press_button(BUTTON_Y, 20, 1000);
    pbf_move_left_joystick(STICK_MAX, STICK_MAX, 6, 500);
    pbf_mash_button(BUTTON_A, 1000);
};

void move_to_location(void) {
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 190, 0);
    pbf_press_button(BUTTON_L, 20, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 200, 200);
    pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 20, 0);
    pbf_press_button(BUTTON_L, 20, 300);
};

void open_box(void) {
    pbf_press_button(BUTTON_A, 20, 700);
    pbf_press_button(BUTTON_X, 20, 400);
    pbf_press_button(BUTTON_A, 20, 700);
};

void get_egg(int boxCol) {
    pbf_press_dpad(DPAD_RIGHT, 20, 400);
    pbf_press_button(BUTTON_MINUS, 20, 400);
    for (int i = 0; i < 40; i++) {
        pbf_press_dpad(DPAD_DOWN, 5, 0);
    };
    pbf_wait(500);
    pbf_press_button(BUTTON_A, 20, 400);
    for (int curCol = 0; curCol < boxCol + 1; curCol++) {
        pbf_press_dpad(DPAD_LEFT, 20, 250);
    };
    pbf_press_dpad(DPAD_DOWN, 20, 400);
    pbf_press_button(BUTTON_A, 20, 400);
    pbf_mash_button(BUTTON_B,600);
    
};

void deposit_pokemon(int boxCol) {
    pbf_press_dpad(DPAD_LEFT, 20, 400);
    pbf_press_dpad(DPAD_DOWN, 20, 400);
    if (HAS_CLONED_RIDER) {
        pbf_press_dpad(DPAD_DOWN, 20, 100);
    };
    pbf_press_button(BUTTON_MINUS, 20, 300);
    for (int i = 0; i < 40; i++) {
        pbf_press_dpad(DPAD_DOWN, 5, 0);
    };
    pbf_wait(400);
    pbf_press_button(BUTTON_A, 20, 400);
    for (int curCol = 0; curCol < boxCol + 1; curCol++) {
        pbf_press_dpad(DPAD_RIGHT, 20, 250);
    };
    pbf_press_dpad(DPAD_UP, 20, 400);
    pbf_press_button(BUTTON_A, 20, 400);
};

void ride_hatch(void) {
    pbf_press_button(BUTTON_PLUS, 20, 500);
    // hatch the first egg at full speed, sometimes the speed boost can fail %point for optimizing%
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 50, 0);
    pbf_press_button(BUTTON_LCLICK, 5, 0);
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, STEPS_TO_HATCH * SAFETY_COEFF, 0);
    // hatch the remaining 4 eggs, not speed boosted to reduce error due to random positioning
    for (int checkEggLoop = 0; checkEggLoop < 90; checkEggLoop++) {
        pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 220, 0);
        pbf_press_button(BUTTON_A, 5, 0);
    };
    pbf_mash_button(BUTTON_A, 750);
    pbf_wait(500);
};

int main(void) {
    start_program_callback();
    initialize_framework(0);

    // ########## Start in grip menu ##########
    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    if (TOLERATE_SYSTEM_UPDATE_MENU_FAST) {
        pbf_press_button(BUTTON_A, 5, 180);
        pbf_move_right_joystick(STICK_CENTER, STICK_MIN, 5, 0);
    };
    pbf_press_button(BUTTON_A, 5, 500);

    for(int curBox = 0; curBox < BOXES_TO_HATCH; curBox++) {
        // ########## Intial loop setup ##########
        open_box();
        pbf_press_dpad(DPAD_LEFT, 20, 200);
        if (HAS_CLONED_RIDER) {
            pbf_press_dpad(DPAD_DOWN, 20, 100);
        };

        // ########## Main hatching loop ##########
        for(int boxCol = 0; boxCol < 6; boxCol++) {
            pbf_wait(500);
            get_egg(boxCol);
            move_to_location();
            ride_hatch();
            fly_to_gate();
            open_box();
            pbf_wait(500);
            deposit_pokemon(boxCol);
        };

        // ########## End loop setup ##########
        pbf_wait(200);
        pbf_press_button(BUTTON_R, 20, 200);
        pbf_press_button(BUTTON_B, 20, 200);
        pbf_press_button(BUTTON_B, 20, 800);
    };
    
    // ########## End idling in home ##########
    pbf_press_button(BUTTON_HOME, 20, 100);
    end_program_callback();
    end_program_loop();
};
