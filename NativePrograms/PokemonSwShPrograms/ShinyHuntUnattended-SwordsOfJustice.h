/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is calibrated for the "average system". If it doesn't
//  work, then you must calibrate it yourself.
//
extern const uint16_t EXIT_CAMP_TO_RUN_DELAY;

//  Set this to true if you are running in airplane mode.
extern const bool AIRPLANE_MODE;


//  These timings are less important.
extern const uint16_t ENTER_CAMP_DELAY;


//  Time Rollback
//
//  Periodically roll back the time to keep the weather the same.
//
//  If set to zero, this feature is disabled.
//  Otherwise, it roll the hour back by this amount every this many hours.
//  This feature requres time-sync be disabled.
extern const uint8_t TIME_ROLLBACK_HOURS;
