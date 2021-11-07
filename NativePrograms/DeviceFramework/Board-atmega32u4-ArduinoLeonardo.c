/*  Arduino Leonardo
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <avr/io.h>
#include "DeviceSettings.h"
#include "HardwareLED.h"

void setup_leds(void){
    DDRB |= (1 << 0);
    DDRD |= (1 << 5);
    DDRC |= (1 << 7);
    onboard_led(false);
}
void onboard_led(bool on){
    if (on){
        PORTB &= ~(1 << 0);
        PORTD &= ~(1 << 5);
        PORTC |= (1 << 7);
    }else{
        PORTB |= (1 << 0);
        PORTD |= (1 << 5);
        PORTC &= ~(1 << 7);
    }
}
void flicker_led(void){
    if (PORTB & 1){
        PORTB &= ~(1 << 0);
        PORTC &= ~(1 << 7);
    }else{
        PORTB |= (1 << 0);
        PORTC |= (1 << 7);
    }
}
