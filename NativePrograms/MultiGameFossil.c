/*  Pokemon Sword & Shield: Beam Resetter
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "PokemonSwShPrograms/MultiGameFossil.h"



//
//  The list of participating saves. Each entry in the array will have 4 parameters:
//
//  Parameter 1:    Game slot at start of program. (must be 1 or 2)
//  Parameter 2:    The slot # of the user. (1 is the left-most user, 2 is the 2nd-to-left, etc...)
//  Parameter 3:    Dracozolt, Arctozolt, Dracovish, or Arctovish
//  Parameter 4:    The number of revives to perform.
//
const Batch BATCH[] = {
    {1, 3, Arctozolt, 960},
    {1, 6, Dracovish, 960},


    //  DO NOT DELETE THIS
    {0},
};


