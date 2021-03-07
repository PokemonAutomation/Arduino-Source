/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  Increase this number if you want to give yourself more time before running.
extern const uint16_t EXIT_CAMP_TO_RUN_DELAY;


//  These two timings are less important.
extern const uint16_t ENTER_CAMP_DELAY;
extern const uint16_t RUN_DELAY;


//  Time Rollback
//
//  Periodically roll back the time to keep the weather the same.
//
//  If set to zero, this feature is disabled.
//  Otherwise, it roll the hour back by this amount every this many hours.
//  This feature requres time-sync be disabled.
extern const uint8_t TIME_ROLLBACK_HOURS;
