/*  Pokemon Sword & Shield: Soft-Reset for Regigigas
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Programs/ShinyHuntUnattended-Regigigas2.h"


//  The amount of Reversal PP that you are saved with.
const uint8_t REVERSAL_PP               =   24;


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is calibrated for the "average system". If it doesn't
//  work, then you must calibrate it yourself.
//
const uint16_t START_TO_ATTACK_DELAY    =   3750;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
const uint32_t TOUCH_DATE_INTERVAL  =   (uint32_t)4 * 3600 * TICKS_PER_SECOND;  //  4 hours


//  Time from when you attack to when you can decide to catch or not catch.
const uint16_t ATTACK_TO_CATCH_DELAY    =   9 * TICKS_PER_SECOND;

//  Time from deciding not to catch to returning to the overworld.
const uint16_t CATCH_TO_OVERWORLD_DELAY =   8 * TICKS_PER_SECOND;
