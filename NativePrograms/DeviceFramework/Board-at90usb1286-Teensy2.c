/*  Teensy++ 2.0
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include <avr/io.h>
#include "DeviceSettings.h"
#include "HardwareLED.h"

const int BOARD_TYPE = BOARD_TYPE_TEENSY2;

void setup_leds(void){
    DDRD |= (1 << 6);
    onboard_led(false);
}
void onboard_led(bool on){
    if (on){
        PORTD |= (1 << 6);
    }else{
        PORTD &= ~(1 << 6);
    }
}
