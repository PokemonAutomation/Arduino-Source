/*  Pokemon Sword & Shield: JPN Date Skipper (7k)
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "PokemonSwShPrograms/DaySkipperJPN.h"



//  The maximum number of skips to perform.
//  The actual # of skips will be lower if any errors are made.
const uint32_t SKIPS = 10;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Run auto-recovery every this # of skips. Zero disables the auto-corrections.
//  At 500, the overhead is approximately 0.5%.
const uint16_t CORRECTION_SKIPS      =   1000;

