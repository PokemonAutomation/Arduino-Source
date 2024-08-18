/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_PushButtons_H
#define PokemonAutomation_NintendoSwitch_PushButtons_H

//#ifdef __AVR__
//#include "NativePrograms/NintendoSwitch/Framework/Master.h"
//#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"



//  Do nothing for the specified number of ticks.
void pbf_wait(uint16_t ticks);

//  Press the specified button for "hold_ticks" ticks.
//  Then release and wait for "release_ticks" ticks.
void pbf_press_button(Button button, uint16_t hold_ticks, uint16_t release_ticks);

//  Press the specified Dpad button for "hold_ticks" ticks.
//  Then release and wait for "release_ticks" ticks.
void pbf_press_dpad(DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);

//  Joystick Positions:
//      Neutral     x = 0x80, y = 0x80
//      Up          x = 0x80, y = 0x00
//      Down        x = 0x80, y = 0xff
//      Left        x = 0x00, y = 0x80
//      Right       x = 0xff, y = 0x80
void pbf_move_left_joystick (uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_right_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);

//  Repeatedly press the specified button as quickly as possible for the
//  specified amount of time.
void pbf_mash_button(Button button, uint16_t ticks);

//  Alternately mash 2 buttons.
void pbf_mash2_button(Button button0, Button button1, uint16_t ticks);



//  Wait this many ticks and flash the LEDs.
void start_program_flash(uint16_t ticks);

//  Connect controller from grip menu and go to Switch home.
void grip_menu_connect_go_home(void);

//  Keep the Switch from sleeping after the program finishes.
//  If the Switch sleeps, it may end up restarting the program!
void end_program_loop(void);



#endif
