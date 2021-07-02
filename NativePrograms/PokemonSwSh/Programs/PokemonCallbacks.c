/*  Device-side Callbacks for the Pokemon Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      All callbacks here MUST return in under 4 milliseconds or it may throw
 *  off the program's clock and break timing-sensitive programs by missing a
 *  Switch USB update.
 * 
 */

#include "PokemonCallbacks.h"

void dayskipper_on_skip_callback(uint32_t remaining_skips){
    //  It is very important this callback returns in under 4 milliseconds.
    //  The day skippers are timing-sensitive and cannot tolerate delays.


}
