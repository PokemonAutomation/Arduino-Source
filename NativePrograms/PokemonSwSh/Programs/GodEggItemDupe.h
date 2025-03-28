/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Fetch this many times.
extern const uint16_t MAX_FETCH_ATTEMPTS;


//  Round-robin through this many party pokemon. By attaching different items
//  to different pokemon in your party, you can duplicate different items in the
//  same run. Thus can clone up to 6 different items in each run.
//
//  If set to 1, it will only clone the item held by the 1st party member.
//  If set to 2, it will alternate cloning items held by the 1st and 2nd party members.
//  If set to N, it will clone the items held by the 1st N members in your party
//  in a round-robin fashion.
//
//  This option doesn't increase the speed of the item duplication. It merely
//  gives you more variety. If you run the program unattended overnight, you may
//  get around 300 of the same item. By setting this parameter, you can instead
//  get 50 of 6 different items.
extern const uint8_t PARTY_ROUND_ROBIN;


//  When you release a pokemon, the item automatically detaches and goes to your
//  inventory. But certain items (like Rusted Sword/Shield) will prevent you
//  from releasing the Pokemon. If you are duplicating such items, you must set
//  this to true.
extern const bool DETACH_BEFORE_RELEASE;


