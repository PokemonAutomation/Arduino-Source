/*  Device Functions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Device_H
#define PokemonAutomation_NintendoSwitch_Device_H

//#ifdef __AVR__
//#include "NativePrograms/NintendoSwitch/Framework/Master.h"
//#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Call this at the start of every program.
void initialize_framework(uint8_t program_id);

//  Turn on/off the LEDs.
void set_leds(bool on);

//  A global clock. Returns the # of ticks that have elapsed since the start of
//  the program.
uint32_t system_clock(void);

void end_program_callback(void);



#endif
