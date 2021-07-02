/*  Pokemon Sword & Shield: God Egg Duplication
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 *  Actually no, you can't refer to the user manual. This program is developer
 *  only since it encourages behavior that we don't want.
 * 
 *  This program is almost identical to GodEggItemDupe, except that it doesn't
 *  detach the items or release the Pokemon.
 * 
 */

#include "Programs/GodEggDuplication.h"



//  Fetch this many times. This puts a limit on how many Pokemon you can clone
//  so you don't make a mess of your boxes for cloning too many.
const uint16_t MAX_FETCH_ATTEMPTS = 2000;



//  Round-robin through this many party pokemon. You can clone up to 6 different
//  Pokemon in the same run.
//
//  If set to 1, it will only clone the 1st party member.
//  If set to 2, it will alternate cloning the 1st and 2nd party members.
//  If set to N, it will clone the 1st N members in your party in a round-robin fashion.
//
//  This option doesn't increase the speed of the duplication. It merely gives
//  you more variety.
const uint8_t PARTY_ROUND_ROBIN     =   6;


