/*  Pokemon Sword & Shield: Trade/Giveaway Bot
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSwShPrograms/TradeBot.h"


//  Raid Code
const char* TRADE_CODE              =   "1280 0000";

const uint8_t BOXES_TO_TRADE        =   2;

//  Set this if you are playing in German.
const bool LINK_TRADE_EXTRA_LINE    =   false;

//  Time to wait for a trade partner.
const uint16_t SEARCH_DELAY         =   20 * TICKS_PER_SECOND;

//  Time to wait for partner to confirm the trade.
const uint16_t CONFIRM_DELAY        =   10 * TICKS_PER_SECOND;


//  Increase these if your internet connection is slow.
const uint16_t TRADE_START          =   10 * TICKS_PER_SECOND;
const uint16_t TRADE_COMMUNICATION  =   20 * TICKS_PER_SECOND;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

const uint16_t TRADE_ANIMATION      =   23 * TICKS_PER_SECOND;
const uint16_t EVOLVE_DELAY         =   30 * TICKS_PER_SECOND;


