/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  Grab watts this many times. You can set this number if you're also date
//  skipping to a particular den frame and you don't want to overshoot it.
//
//  Be aware that this program isn't intended to be an accurate date skipper.
//  It will occasionally miss frames causing it to fall short.
extern const uint32_t SKIPS;


//  Save the game every this number of iterations.
//  If set to zero, no saving is done.
extern const uint16_t SAVE_ITERATIONS;


