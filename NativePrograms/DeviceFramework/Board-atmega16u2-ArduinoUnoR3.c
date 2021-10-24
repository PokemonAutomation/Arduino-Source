/*  Arduino Uno R3
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
void flicker_led(void){
    uint8_t port = PORTD;
    
    port ^= (port ^ (port << 1)) & (1 << 5);
    port ^= (1 << 4);

    PORTD = port;
}
