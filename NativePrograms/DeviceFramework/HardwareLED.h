/*  Onboard LEDs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_HardwareLED_H
#define PokemonAutomation_HardwareLED_H

#include <stdbool.h>

void setup_leds(void);
void onboard_led(bool on);
void flicker_led(void);


#endif
