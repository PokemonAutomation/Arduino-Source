/*  Arduino Uno R3
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include <avr/io.h>
#include "DeviceSettings.h"
#include "HardwareLED.h"

const int BOARD_TYPE = BOARD_TYPE_UNO;

void setup_leds(void){
    const int leds = (1 << 4) | (1 << 5);
    DDRD  |= leds;
    PORTD |= leds;
}
void onboard_led(bool on){
    const int leds = (1 << 4) | (1 << 5);
    if (on){
        PORTD &= ~leds;
    }else{
        PORTD |= leds;
    }
}
