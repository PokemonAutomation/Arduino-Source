/*  Device Settings
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "DeviceSettings.h"
#include "HardwareLED.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  User Callbacks

void start_program_callback(void){
    //  This is run when the program starts. Here you can insert code to
    //  setup output pins.

    //  This callback should return quickly. Do not hang in here for too long
    //  or it will delay the start of the program.

    setup_leds();
}
void end_program_callback(void){
    //  This is run when the program finishes. Here you can insert code to
    //  turn on LEDs or other output pins.

    //  This callback should return quickly. Do not hang in here for too long
    //  or it will prevent the program from entering its safe end-loop.

    onboard_led(true);
}

