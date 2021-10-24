/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Run Loto-ID this many times. You can set this number if you're also date
//  skipping to a particular den frame and you don't want to overshoot it.
//
//  Be aware that this program isn't intended to be an accurate date skipper.
//  It will occasionally miss frames causing it to fall short.
extern const uint32_t SKIPS;

//  Mash B for this long to exit the dialog.
//  For some languages, (like German), you may need to increase this.
extern const uint16_t MASH_B_DURATION;

