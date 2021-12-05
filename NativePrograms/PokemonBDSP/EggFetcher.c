/*  Pokemon BDSP Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Programs/EggFetcher.h"


//  Bike shortcut direction.
const ShortcutDirection BIKE_SHORTCUT_DIRECTION = SHORTCUT_UP;



//  Fetch this many times. This puts a limit on how many eggs you can get so
//  you don't make a mess of your boxes for fetching too many.
const uint16_t MAX_FETCH_ATTEMPTS = 2000;


//  Fetch an egg after traveling for this long.
const uint16_t TRAVEL_TIME_PER_FETCH    =   15 * TICKS_PER_SECOND;



