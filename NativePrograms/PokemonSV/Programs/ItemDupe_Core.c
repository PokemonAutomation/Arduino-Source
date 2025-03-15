// SCVI ITEM DUPLICATION
// ---------------------
// Includes reset in case of failures
//
// CREDITS
// -------
// - Pokemon Automation Team for HEX Generator, Program Base, and Help in development
// - Vaedz#1011 for Original Program, Development, Failure Reset Function, and Startup Correction
// - Zeverand#6019 for Modifications on timing and Dpad Controls
// - Jack Mutsers#2563 for Optimization, Original Violet Program, Development, Joystick Reset for Emotes, Joystick Cycle System, and Program Condesation
// -------
// SCARLET AND VIOLET VERSION
// WORKS WITH BOTH GAMES
//
// TESTING
// -------
// Teensy 2 - STABLE
// Teensy 2++ - STABLE
// Arduino Leonardo - STABLE
// Arduino UNO R3 - STABLE
// Pro Micro - STABLE

#include "NativePrograms/DeviceFramework/DeviceSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/FrameworkSettings.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_Device.h"
#include "NativePrograms/NintendoSwitch/Libraries/NintendoSwitch_PushButtons.h"
#include "NativePrograms/PokemonSV/Programs/ItemDupe.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSwSh_Settings.h"

int resetFail = 0;
int incConst = 1;
bool oddCycle = true;

int main(void){
// START PROGRAM ON MICROCONTROLLER
start_program_callback();
initialize_framework(0);

// START PROGRAM ON SWITCH
start_program_flash(CONNECT_CONTROLLER_DELAY);
grip_menu_connect_go_home();

// START GAME (BE SURE TO HAVE GLITCH ACTIVE)
pbf_press_button(BUTTON_HOME, 10, 375);
    
// ENSURE MENU IS IN CORRECT POSITION
pbf_press_button(BUTTON_X, 5, 5);
pbf_wait(100);
pbf_press_dpad(DPAD_LEFT, 5, 5);
pbf_wait(100);
pbf_press_dpad(DPAD_UP, 500, 5);
pbf_wait(100);
pbf_press_button(BUTTON_X, 5, 5);

while (true){
    // DELAY START
    pbf_wait(50);

    // ENTER MENU AND NAVIGATE TO LEGENDARY
    pbf_press_button(BUTTON_X, 5, 5);

    // ACTIVATE RIDE FORM GLITCH
    pbf_wait(160);
    pbf_press_button(BUTTON_A, 5, 125);
    pbf_press_dpad(DPAD_UP, 5, 10);
    pbf_press_dpad(DPAD_UP, 5, 85);
    pbf_press_button(BUTTON_A, 5, 130);
    pbf_press_button(BUTTON_A, 5, 140);
    pbf_press_button(BUTTON_A, 5, 100); // ENSURE DIALOG ENDS
    pbf_press_button(BUTTON_A, 5, 300);
    pbf_press_button(BUTTON_A, 5, 100);

    // COLLECT ITEM FROM BOX
    pbf_press_dpad(DPAD_RIGHT, 5, 85);
    pbf_press_dpad(DPAD_DOWN, 5, 25);
    pbf_press_dpad(DPAD_DOWN, 5, 55);
    pbf_press_button(BUTTON_A, 5, 420);
    pbf_press_button(BUTTON_X, 5, 60);
    pbf_press_button(BUTTON_X, 5, 60);
    pbf_press_button(BUTTON_L, 5, 80);
    pbf_press_button(BUTTON_A, 5, 100);
    pbf_press_dpad(DPAD_DOWN, 5, 10);
    pbf_press_dpad(DPAD_DOWN, 5, 10);
    pbf_press_dpad(DPAD_DOWN, 5, 65);
    pbf_press_button(BUTTON_A, 5, 100);
    pbf_press_button(BUTTON_B, 5, 240);

    // RESET MENU AND NAVIGATE OUT OF MENU
    pbf_press_dpad(DPAD_LEFT, 5, 10);
    pbf_press_dpad(DPAD_LEFT, 5, 80);
    pbf_press_dpad(DPAD_UP, 5, 50);
    pbf_press_button(BUTTON_B, 5, 20);
    pbf_press_button(BUTTON_B, 5, 5); // ENSURE MENU IS CLOSED
    pbf_wait(130);

    // CHECK FAILURE EVERY 4 CYCLES
    if (resetFail > 2) {
        pbf_press_button(BUTTON_B, 5, 5);
        pbf_wait(60);
        pbf_press_button(BUTTON_B, 5, 5);
        pbf_wait(60);
        pbf_press_button(BUTTON_B, 5, 5);
        pbf_wait(60);
        pbf_press_button(BUTTON_B, 5, 5);
        pbf_wait(80);

    // ENABLE EMOTE ABILITY (MOVEMENT LOCK)
        pbf_press_dpad(DPAD_RIGHT, 5, 5);
        pbf_wait(170);
        pbf_press_button(BUTTON_B, 5, 5);
        pbf_wait(90);

    // PREVENT CHARACTER FROM WALKING AWAY
        if(oddCycle){
            pbf_move_left_joystick(0x80, 0x00, 1, 5);
        }else{
            pbf_move_left_joystick(0x80, 0xff, 1, 5);
        }

        pbf_wait(60);
        pbf_press_button(BUTTON_X, 5, 5);
        pbf_wait(100);
        pbf_press_dpad(DPAD_LEFT, 5, 5);
        pbf_wait(80);
        pbf_press_dpad(DPAD_UP, 500, 5);
        pbf_wait(50);
        pbf_press_button(BUTTON_B, 5, 5);
        resetFail = 0;
        oddCycle = !oddCycle;
    } else {
        resetFail = resetFail + incConst;
    };
}
    //EXIT TO IDLE MODE (NEVER USED)
    pbf_press_button(BUTTON_HOME, 10, 125);
    end_program_callback();
    end_program_loop();
};
