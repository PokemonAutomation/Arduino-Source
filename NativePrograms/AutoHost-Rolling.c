/*  Pokemon Sword & Shield: Rolling Auto-Host with Friend-Request Accepts
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "PokemonSwShPrograms/AutoHost-Rolling.h"



//  Raid Code
//      No code:            RANDOM_DIGITS = 0       RAID_CODE = ""
//      Fixed code:         RANDOM_DIGITS = 0       RAID_CODE = "1234 5678"
//      Random Code:        RANDOM_DIGITS = 4       RAID_CODE = "1234 5678"
//
//  Non-digit characters (such as spaces or hyphen) will be skipped.
//
//  "RANDOM_DIGITS" specifies how many of the first digits are randomized.
//  If it is less than 8, the remaining digits will be copies of the last digit.
//  This makes it easier to enter the code.
//
//  When using random codes, "RAID_CODE" is used to generate the starting seed.
const uint8_t RANDOM_DIGITS     =   0;
const char* RAID_CODE           =   "1280 0000";


//  General Settings
const uint8_t SKIPS             =   3;                      //  Set to 0 if hard-locked on desired frame.
const bool HOST_ONLINE          =   true;                   //  true = host online, false = host offline
const uint16_t LOBBY_WAIT_DELAY =   60 * TICKS_PER_SECOND;  //  3:00 - 60 secs = 2:00


//  Backup Save: (for soft-lock hosting)
//      If set to true, the game will load backup save upon soft-reset.
//  The idea here is to roll forward from the save frame to the shiny frame.
//  Once on the shiny frame (with the desired softlock), you enter and exit the
//  camp to setup a backup save on the shiny frame. That way, you can load the
//  backup save on the shiny frame while leaving the real save safely 3 or more
//  frames before.
//
//  This option only makes sense when SKIPS = 0.
const bool BACKUP_SAVE      =   false;


//  ALWAYS_CATCHABLE    All pokemon in this den are catchable.
//  MAYBE_UNCATCHABLE   There may be uncatchable pokemon.
const Catchability CATCHABILITY     =   ALWAYS_CATCHABLE;


//  Friend-Request Accepting:
//      0   =   Disabled. Do not automatically accept friend requests.
//      #   =   Automatically accept friend requests for the N'th user.
//
//  This number is the slot position of the user at the top of the Home menu.
//  The left-most user is slot 1. This option only works if "HOST_ONLINE" is true.
//
//  Friend accepting is done while waiting in the raid lobby. Therefore it does
//  not slow down the program.
const uint8_t FRIEND_ACCEPT_USER_SLOT       =   0;


//  Additional Raid Delay:
//      If you are farming a pokemon and the time between raids is too short to
//  join consecutive raids, use this to add time between raids. This extra wait
//  time is done after entering the den, but before entering the code.
const uint16_t EXTRA_DELAY_BETWEEN_RAIDS    =   0 * TICKS_PER_SECOND;


//  Select 1st Move:
//      MOVE_SLOT   =   0   Do not select a move for first turn.
//      MOVE_SLOT   =   #   Select this move for the first turn.
const uint8_t MOVE_SLOT     =   0;
const bool DYNAMAX          =   true;   //  Must set to false if you cannot dmax.


//  Alternate Games: (experimental feature)
//      If set to true, alternate between games 1 and 2.
//  Basically if you have both Sword and Shield on the same user and you wish
//  to host dens on both with the same settings. This implies that at least of
//  the games must be a digital copy since you cannot have 2 carts in at once.
const bool ALTERNATE_GAMES  =   false;


//  Rollover Prevention
//
//  This option is only relevant if (SKIPS = 0).
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent the den from rolling over. This feature requres time-sync be disabled.
const uint32_t TOUCH_DATE_INTERVAL  =   (uint32_t)4 * 3600 * TICKS_PER_SECOND;  //  4 hours



//  Increase these if your internet is slow.
const uint16_t CONNECT_TO_INTERNET_DELAY    =   20 * TICKS_PER_SECOND;  //  Time from "Connect to Internet" to when you're ready to enter den.
const uint16_t ENTER_ONLINE_DEN_DELAY       =   8 * TICKS_PER_SECOND;   //  "Communicating" when entering den while online.
const uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY  =   8 * TICKS_PER_SECOND;   //  Delay from "Invite Others" to when the clock starts ticking.

const uint16_t RAID_START_TO_EXIT_DELAY     =   15 * TICKS_PER_SECOND;  //  Time from start raid to reset. (when not selecting move)
const uint16_t DELAY_TO_SELECT_MOVE         =   32 * TICKS_PER_SECOND;  //  This + RAID_START_TO_EXIT_DELAY = time from start raid to select move.


