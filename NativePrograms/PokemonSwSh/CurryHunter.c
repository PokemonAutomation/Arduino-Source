/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

// This is a program that cooks curry for you and catch any Pokémon found while doing so.


#include "Programs/CurryHunter.h"

//  Wait this long for the Pokemon to walk up to you.
const uint16_t WALK_UP_DELAY = 2 * TICKS_PER_SECOND;

//  Take a video after each cooking iteration. This will spam your album with videos.
const bool TAKE_VIDEO = false;

//  Run this many iterations before stopping.
const uint32_t ITERATIONS = 999;
