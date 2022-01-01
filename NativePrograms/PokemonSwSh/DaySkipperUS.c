/*  Pokemon Sword & Shield: US Date Skipper (7k)
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 *  This is the US version of the 7k skipper that at about 7100 skips/hour.
 * 
 *  This program is slower than the JPN skipper. So if you're doing long skips,
 *  you'll still want to change your Switch language to Japanese.
 * 
 */

#include "Programs/DaySkipperUS.h"



//  The maximum number of skips to perform.
//  The actual # of skips will be lower if any errors are made.
const uint32_t SKIPS            =   10;

//  The current year in real life.
const uint16_t REAL_LIFE_YEAR   =   2022;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Run auto-recovery every this # of skips. Zero disables the auto-corrections.
//  At 500, the overhead is approximately 0.5%.
const uint16_t CORRECTION_SKIPS      =   500;

