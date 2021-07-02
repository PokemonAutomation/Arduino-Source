/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  Set to a large number to delete all friends.
extern const uint16_t FRIENDS_TO_DELETE;

//  Instead of deleting friends, block them.
extern const bool BLOCK_FRIENDS;


//  Timings
extern const uint16_t VIEW_FRIEND_DELAY;    //  Delay from opening a friend to when you can press buttons.
extern const uint16_t DELETE_FRIEND_DELAY;  //  Delay to delete the friend.
extern const uint16_t FINISH_DELETE_DELAY;  //  Delay after deleting a friend.

