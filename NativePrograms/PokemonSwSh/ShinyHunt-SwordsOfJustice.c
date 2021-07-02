/*  Pokemon Sword & Shield: Soft-Reset for Swords of Justice
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Programs/ShinyHunt-SwordsOfJustice.h"

//  Increase this number if you want to give yourself more time before running.
const uint16_t EXIT_CAMP_TO_RUN_DELAY   =   19 * TICKS_PER_SECOND;


//  Time Rollback
//
//  Periodically roll back the time to keep the weather the same.
//
//  If set to zero, this feature is disabled.
//  Otherwise, it roll the hour back by this amount every this many hours.
//  This feature requres time-sync be disabled.
const uint8_t TIME_ROLLBACK_HOURS   =   1;


//  These two timings are less important.
const uint16_t ENTER_CAMP_DELAY         =   8 * TICKS_PER_SECOND;
const uint16_t RUN_DELAY                =   5 * TICKS_PER_SECOND;
