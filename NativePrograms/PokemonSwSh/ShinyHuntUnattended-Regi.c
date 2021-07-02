/*  Pokemon Sword & Shield: Soft-Reset for Regi Golems
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Programs/ShinyHuntUnattended-Regi.h"


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is calibrated for the "average system". If it doesn't
//  work, then you must calibrate it yourself.
//
const uint16_t START_TO_RUN_DELAY   =   1990;

//  Name of the Regi you are SR'ing for.
//
//  Available Options:
//      -   Regirock
//      -   Regice
//      -   Registeel
//      -   Regieleki
//      -   Regidrago
//
const RegiGolem REGI_NAME           =   Registeel;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
const uint32_t TOUCH_DATE_INTERVAL  =   (uint32_t)4 * 3600 * TICKS_PER_SECOND;  //  4 hours



//  The light puzzle is not 100% reliable. Sometimes it will miss lights.
//  To counter that, the program will periodically leave and re-enter the
//  building after this many encounters. Zero disables the corrections.
//  If you disable corrections, the program will stop working when it
//  misses a light and will not self-recover.
const uint16_t CORRECTION_INTERVAL  =   20;

//  Delay to enter/exit the building.
const uint16_t TRANSITION_DELAY     =   5 * TICKS_PER_SECOND;
