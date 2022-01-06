/*  Pokemon BDSP Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Bike shortcut direction.
typedef enum{
    SHORTCUT_UP,
    SHORTCUT_RIGHT,
    SHORTCUT_DOWN,
    SHORTCUT_LEFT,
} ShortcutDirection;
extern const ShortcutDirection BIKE_SHORTCUT_DIRECTION;



//  Fetch this many times. This puts a limit on how many eggs you can get so
//  you don't make a mess of your boxes for fetching too many.
extern const uint16_t MAX_FETCH_ATTEMPTS;


//  Fetch an egg after traveling for this long.
extern const uint16_t TRAVEL_TIME_PER_FETCH;



