/*  Pokemon Sword & Shield: Soft-Reset for Strong Spawn
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Programs/ShinyHuntUnattended-StrongSpawn.h"


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is unlikely to work for your setup since there are
//  so many factors involved. You must calibrate it for your particular setup.
//
const uint16_t ENTER_GAME_TO_RUN_DELAY  =   2280;


//  Time Rollback
//
//  Periodically roll back the time to keep the weather the same.
//
//  If set to zero, this feature is disabled.
//  Otherwise, it roll the hour back by this amount every this many hours.
//  This feature requres time-sync be disabled.
const uint8_t TIME_ROLLBACK_HOURS   =   1;



//  Wait this long for the game to start before pressing A to enter the game
//  and start the encounter. You can try to decrease this to make resetting
//  faster.
const uint16_t START_GAME_WAIT_DELAY    =   20 * TICKS_PER_SECOND;
