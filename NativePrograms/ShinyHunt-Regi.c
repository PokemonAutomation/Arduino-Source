/*  Pokemon Sword & Shield: Soft-Reset for Regis
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSwShPrograms/ShinyHunt-Regi.h"

const uint16_t WAIT_TIME    =   15 * TICKS_PER_SECOND;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
const uint32_t TOUCH_DATE_INTERVAL  =   (uint32_t)4 * 3600 * TICKS_PER_SECOND;  //  4 hours
