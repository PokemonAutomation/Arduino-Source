/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  The amount of Reversal PP that you are saved with.
extern const uint8_t REVERSAL_PP;


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is calibrated for the "average system". If it doesn't
//  work, then you must calibrate it yourself.
//
extern const uint16_t START_TO_ATTACK_DELAY;


//  Time from when you attack to when you can decide to catch or not catch.
extern const uint16_t ATTACK_TO_CATCH_DELAY;

//  Time from deciding not to catch to returning to the overworld.
extern const uint16_t CATCH_TO_OVERWORLD_DELAY;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
extern const uint32_t TOUCH_DATE_INTERVAL;
