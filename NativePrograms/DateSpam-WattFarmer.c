/*  Pokemon Sword & Shield: Watt Farmer
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "PokemonSwShPrograms/DateSpam-WattFarmer.h"



//  Grab watts this many times. You can set this number if you're also date
//  skipping to a particular den frame and you don't want to overshoot it.
//
//  Be aware that this program isn't intended to be an accurate date skipper.
//  It will occasionally miss frames causing it to fall short.
const uint32_t SKIPS            =   33334;


//  Save the game every this number of iterations.
//  If set to zero, no saving is done.
const uint16_t SAVE_ITERATIONS  =   0;
