/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  Fetch this many times. This puts a limit on how many Pokemon you can clone
//  so you don't make a mess of your boxes for cloning too many.
extern const uint16_t MAX_FETCH_ATTEMPTS;


//  Round-robin through this many party pokemon. You can clone up to 6 different
//  Pokemon in the same run.
//
//  If set to 1, it will only clone the 1st party member.
//  If set to 2, it will alternate cloning the 1st and 2nd party members.
//  If set to N, it will clone the 1st N members in your party in a round-robin fashion.
//
//  This option doesn't increase the speed of the duplication. It merely gives
//  you more variety.
extern const uint8_t PARTY_ROUND_ROBIN;


