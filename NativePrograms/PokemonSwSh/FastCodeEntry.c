/*  Pokemon Sword & Shield: Fast Code Entry
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Programs/FastCodeEntry.h"



//  Raid Code
const char* RAID_CODE = "9107 3091";

//  Wait this long before entering the code.
const uint16_t INITIAL_DELAY = 0 * TICKS_PER_SECOND;
