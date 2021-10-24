/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Raid Code
extern const char* TRADE_CODE;

extern const uint8_t BOXES_TO_TRADE;

//  Set this if you are playing in German.
extern const bool LINK_TRADE_EXTRA_LINE;

//  Time to wait for a trade partner.
extern const uint16_t SEARCH_DELAY;

//  Time to wait for partner to confirm the trade.
extern const uint16_t CONFIRM_DELAY;


//  Increase these if your internet connection is slow.
extern const uint16_t TRADE_START;
extern const uint16_t TRADE_COMMUNICATION;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

extern const uint16_t TRADE_ANIMATION;
extern const uint16_t EVOLVE_DELAY;


