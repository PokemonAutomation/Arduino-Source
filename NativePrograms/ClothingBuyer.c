/*  Pokemon Sword & Shield: Clothing Buyer
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "PokemonSwShPrograms/ClothingBuyer.h"


//  Rotate categories. This will allow the program to eventually buy out the
//  entire store, but it will take a lot longer than doing separate runs on
//  each category individually.
const bool CATEGORY_ROTATION    =   true;
