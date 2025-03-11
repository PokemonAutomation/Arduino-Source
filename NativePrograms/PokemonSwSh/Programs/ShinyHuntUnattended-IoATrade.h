/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run on a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is calibrated for the "average system". If it doesn't
//  work, then you must calibrate it yourself.
//
extern const uint16_t START_TO_RUN_DELAY;




//
//  You shouldn't need to touch anything below here.
//


//  Time it takes to fly from IoA to Route 10.
extern const uint16_t FLY_DURATION;

//  Time to move from Route 10 fly spot to the Beartic.
extern const uint16_t MOVE_DURATION;

//  Time to perform the trade.
extern const uint16_t MASH_TO_TRADE_DELAY;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
extern const uint32_t TOUCH_DATE_INTERVAL;
