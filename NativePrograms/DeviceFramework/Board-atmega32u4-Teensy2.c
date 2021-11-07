/*  Teensy 2.0
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <avr/io.h>
#include "DeviceSettings.h"
#include "HardwareLED.h"

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
void flicker_led(void){
    PORTD ^= (1 << 6);
}
