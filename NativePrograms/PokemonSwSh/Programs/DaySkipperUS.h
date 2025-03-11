/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  The maximum number of skips to perform.
//  The actual # of skips will be lower if any errors are made.
extern const uint32_t SKIPS;

//  The current year in real life.
extern const uint16_t REAL_LIFE_YEAR;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Run auto-recovery every this # of skips. Zero disables the auto-corrections.
//  At 500, the overhead is approximately 0.5%.
extern const uint16_t CORRECTION_SKIPS;


