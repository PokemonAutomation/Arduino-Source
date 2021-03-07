/*  Pokemon Sword & Shield: Surprise Trader
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSwShPrograms/SurpriseTrade.h"



const uint8_t BOXES_TO_TRADE    =   2;

//  Time to wait for a trade partner.
const uint16_t INITIAL_WAIT     =   30 * TICKS_PER_SECOND;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

const uint16_t TRADE_ANIMATION  =   23 * TICKS_PER_SECOND;
const uint16_t EVOLVE_DELAY     =   30 * TICKS_PER_SECOND;


