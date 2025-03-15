
#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "EggCombined.h"

void open_picnic(void) {
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 190, 0);
    pbf_press_button(BUTTON_L, 20, 80);
    pbf_press_button(BUTTON_X, 20, 200);
    pbf_press_button(BUTTON_A, 20, 1000);
};

void move_to_basket(void) {
    pbf_press_button(BUTTON_L, 20, 100);
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 50, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 70, 0);
    pbf_press_button(BUTTON_L, 20, 100);
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 50, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 100, 50);
    pbf_press_button(BUTTON_L, 20, 100);
    pbf_press_button(BUTTON_PLUS, 20, 200);
};

void fly_to_gate(void) {
    pbf_press_button(BUTTON_A, 20, 500);
    pbf_press_button(BUTTON_Y, 20, 800);
    pbf_move_left_joystick(STICK_MAX, STICK_MAX, 6, 500);
    pbf_mash_button(BUTTON_A, 1000);
};

void move_to_location(void) {
    pbf_move_left_joystick(STICK_MIN, STICK_CENTER, 190, 0);
    pbf_press_button(BUTTON_L, 20, 0);
    pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 150, 100);
    pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 20, 0);
    pbf_press_button(BUTTON_L, 20, 300);
};

void open_box(void) {
    pbf_press_button(BUTTON_A, 20, 500);
    pbf_press_button(BUTTON_X, 20, 200);
    pbf_press_button(BUTTON_A, 20, 500);
};

void get_egg(int boxCol) {
    pbf_press_dpad(DPAD_RIGHT, 20, 100);
    pbf_press_button(BUTTON_MINUS, 20, 100);
    for (int i = 0; i < 40; i++) {
        pbf_press_dpad(DPAD_DOWN, 5, 0);
    };
    pbf_wait(100);
    pbf_press_button(BUTTON_A, 20, 100);
    for (int curCol = 0; curCol < boxCol + 1; curCol++) {
        pbf_press_dpad(DPAD_LEFT, 20, 100);
    };
    pbf_press_dpad(DPAD_DOWN, 20, 100);
    pbf_press_button(BUTTON_A, 20, 100);
    pbf_press_button(BUTTON_B, 20, 200);
    pbf_press_button(BUTTON_B, 20, 800);
};

void deposit_pokemon(int boxCol) {
    pbf_press_dpad(DPAD_LEFT, 20, 100);
    pbf_press_dpad(DPAD_DOWN, 20, 100);
    if (HAS_CLONED_RIDER) {
        pbf_press_dpad(DPAD_DOWN, 20, 100);
    };
    pbf_press_button(BUTTON_MINUS, 20, 100);
    for (int i = 0; i < 40; i++) {
        pbf_press_dpad(DPAD_DOWN, 5, 0);
    };
    pbf_wait(100);
    pbf_press_button(BUTTON_A, 20, 100);
    for (int curCol = 0; curCol < boxCol + 1; curCol++) {
        pbf_press_dpad(DPAD_RIGHT, 20, 100);
    };
    pbf_press_dpad(DPAD_UP, 20, 100);
    pbf_press_button(BUTTON_A, 20, 100);
};

void ride_hatch(void) {
    pbf_press_button(BUTTON_PLUS, 20, 400);
    // hatch the first egg at full speed, sometimes the speed boost can fail %point for optimizing%
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 50, 0);
    pbf_press_button(BUTTON_LCLICK, 5, 0);
    pbf_move_left_joystick(STICK_MAX, STICK_CENTER, STEPS_TO_HATCH * SAFETY_COEFF, 0);
    // hatch the remaining 4 eggs, not speed boosted to reduce error due to random positioning
    for (int checkEggLoop = 0; checkEggLoop < 80; checkEggLoop++) {
        pbf_move_left_joystick(STICK_MAX, STICK_CENTER, 220, 0);
        pbf_press_button(BUTTON_A, 5, 0);
    };
};

void deposit_parent(void) {
    pbf_press_button(BUTTON_X, 20, 200);
    pbf_press_dpad(DPAD_UP, 20, 100);
    pbf_press_button(BUTTON_A, 20, 500);
    pbf_press_button(BUTTON_L, 20, 200);
    pbf_press_button(BUTTON_Y, 20, 500);
    pbf_press_dpad(DPAD_LEFT, 20, 100);
    pbf_press_button(BUTTON_Y, 20, 500);
    pbf_press_dpad(DPAD_DOWN, 20, 100);
    if (HAS_CLONED_RIDER) {
        pbf_press_dpad(DPAD_DOWN, 20, 100);
    };
    pbf_press_button(BUTTON_MINUS, 20, 100);
    for (int i = 0; i < 40; i++) {
        pbf_press_dpad(DPAD_DOWN, 5, 0);
    };
    pbf_wait(100);
    pbf_press_button(BUTTON_A, 20, 100);
    pbf_press_dpad(DPAD_RIGHT, 20, 100);
    pbf_press_dpad(DPAD_RIGHT, 20, 100);
    pbf_press_dpad(DPAD_UP, 20, 100);
    pbf_press_button(BUTTON_A, 20, 100);
    pbf_press_button(BUTTON_R, 20, 200);
    pbf_press_button(BUTTON_B, 20, 200);
    pbf_press_button(BUTTON_B, 20, 800);
};

int main(void) {
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

    // ########## Egg Fetching ##########
    open_picnic();
    move_to_basket();

    for (int curSand = 0; curSand < MAX_SAND; curSand++) {
        pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 20, 100);
        // START SANDWICH
        pbf_press_button(BUTTON_A, 20, 250);
        pbf_press_button(BUTTON_A, 20, 750);
        // SELECT RECIPE
        if (USE_HM) {
            pbf_press_dpad(DPAD_DOWN, 20, 100);
        }
        pbf_press_button(BUTTON_A, 20, 250);
        // SELECT PICK
        pbf_press_button(BUTTON_A, 20, 1250);
        // MAKE SANDWICH
        pbf_move_left_joystick(STICK_CENTER, STICK_MIN, 250, 100);
        pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 25, 100);
        pbf_mash_button(BUTTON_A, 1500);
        pbf_wait(3000);
        pbf_press_button(BUTTON_A, 20, 150);
        // WAIT AT BASKET
        pbf_move_left_joystick(STICK_CENTER, STICK_MAX, 5, 100);
        for (int rounds = 0; rounds < 10; rounds++) {
            pbf_wait(22500);
            pbf_press_button(BUTTON_A, 20, 100);
            pbf_mash_button(BUTTON_B, 4000);
        };
        pbf_wait(200);
    };

    // ########## Transition ##########
    pbf_press_button(BUTTON_Y, 20, 150);
    pbf_press_button(BUTTON_A, 20, 800);

    deposit_parent();
    fly_to_gate();

    // ########## Egg Hatching ##########
    for(int curBox = 0; curBox < BOXES_TO_HATCH; curBox++) {
        // ########## Intial loop setup ##########
        open_box();
        pbf_press_dpad(DPAD_LEFT, 20, 200);
        if (HAS_CLONED_RIDER) {
            pbf_press_dpad(DPAD_DOWN, 20, 100);
        };

        // ########## Main hatching loop ##########
        for(int boxCol = 0; boxCol < 6; boxCol++) {
            get_egg(boxCol);
            move_to_location();
            ride_hatch();
            fly_to_gate();
            open_box();
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