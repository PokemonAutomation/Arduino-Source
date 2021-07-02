/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <avr/pgmspace.h>
#include "Common/SwitchFramework/SwitchControllerDefs.h"


typedef enum{
    Dracozolt   =   0,
    Arctozolt   =   1,
    Dracovish   =   2,
    Arctovish   =   3,
} FossilMon;
typedef struct{
    uint8_t game_slot : 2;
    uint8_t user_slot : 4;
    FossilMon mon : 2;
    uint16_t revives;
} Batch;


//
//  The list of participating saves. Each entry in the array will have 4 parameters:
//
//  Parameter 1:    Game slot at start of program. (must be 1 or 2)
//  Parameter 2:    The slot # of the user. (1 is the left-most user, 2 is the 2nd-to-left, etc...)
//  Parameter 3:    The pokemon you are reviving in this save.
//  Parameter 4:    The number of revives to perform.
//
extern const Batch PROGMEM BATCH[];


