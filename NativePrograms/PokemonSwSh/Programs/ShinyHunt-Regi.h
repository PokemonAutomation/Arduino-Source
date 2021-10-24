/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


extern const uint16_t WAIT_TIME;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
extern const uint32_t TOUCH_DATE_INTERVAL;

