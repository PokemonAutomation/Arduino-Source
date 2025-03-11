/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  This needs to be carefully calibrated.
//    - If it's too short, it may stop on a non-shiny.
//    - If it's too long, it may run from a shiny.
//
//  There is less than a 0.5 second window which this must land in.
//
//  The default value here is unlikely to work for your setup since there are
//  so many factors involved. You must calibrate it for your particular setup.
//
extern const uint16_t ENTER_GAME_TO_RUN_DELAY;



//  Wait this long for the game to start before pressing A to enter the game
//  and start the encounter. You can try to decrease this to make resetting
//  faster.
extern const uint16_t PokemonSwSh_START_GAME_WAIT_DELAY;


//  Time Rollback
//
//  Periodically roll back the time to keep the weather the same.
//
//  If set to zero, this feature is disabled.
//  Otherwise, it roll the hour back by this amount every this many hours.
//  This feature requres time-sync be disabled.
extern const uint8_t TIME_ROLLBACK_HOURS;

