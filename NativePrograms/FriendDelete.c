/*  Pokemon Sword & Shield: Friend Deleter
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSwShPrograms/FriendDelete.h"



//  Set to a large number to delete all friends.
const uint16_t FRIENDS_TO_DELETE    =   3;

//  Instead of deleting friends, block them.
const bool BLOCK_FRIENDS            =   false;


//  Timings: You may need to increase if your internet is slow.
const uint16_t VIEW_FRIEND_DELAY    =   2 * TICKS_PER_SECOND;   //  Delay from opening a friend to when you can press buttons.
const uint16_t DELETE_FRIEND_DELAY  =   8 * TICKS_PER_SECOND;   //  Delay to delete the friend.
const uint16_t FINISH_DELETE_DELAY  =   2 * TICKS_PER_SECOND;   //  Delay after deleting a friend.


