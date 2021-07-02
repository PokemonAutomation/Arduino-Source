/*  Pokemon Sword & Shield: JPN Date Skipper (7.8k)
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 *  This program is experimental. Instructions are the same as DaySkipperJPN
 *  except that you must set the date to January 1, 2000. The hour must be set
 *  away from the DST missing hour.
 * 
 *  This program will run at ~7800 skips/hour at full speed. It will be able to
 *  tolerate single missed skips and maintain this full speed. However, if a
 *  large error happens (such as a trapping error), it will recover, but it will
 *  only be able to run at a degraded state of ~7500/hour.
 * 
 */

#include "Programs/DaySkipperJPN-7.8k.h"



//  The maximum number of skips to perform.
//  The actual # of skips will be lower if any errors are made.
const uint32_t SKIPS = 10;

//  If you don't want to start on 1/1/2000, you can set this instead.
const Date START_DATE = {2000, 1, 1};  //  {year, month, day}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Run auto-recovery every this # of skips. Zero disables the auto-corrections.
//  At 500, the overhead is approximately 0.5%.
const uint16_t CORRECTION_SKIPS      =   1000;

