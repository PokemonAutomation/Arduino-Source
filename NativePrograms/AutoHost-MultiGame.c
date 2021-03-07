/*  Pokemon Sword & Shield: Multi-Game Rolling Auto-Host
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSwShPrograms/AutoHost-MultiGame.h"



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
const bool HOST_ONLINE          =   true;                   //  true = host online, false = host offline
const uint16_t LOBBY_WAIT_DELAY =   60 * TICKS_PER_SECOND;  //  3:00 - 60 secs = 2:00


//  Game List
const RollingAutoHostSlot GAME_LIST[] = {
    {
        .game_slot          =   1,                      //  Game slot at start of program. (must be 1 or 2)
        .user_slot          =   2,                      //  User # to host from.
        .skips              =   3,                      //  # of frame skips to roll before hosting.
        .backup_save        =   false,                  //  Load backup save instead of normal reset.
        .always_catchable   =   true,                   //  Pokemon in this den are always catchable.
        .accept_FRs         =   true,                   //  Accept friend requests.
        .move_slot          =   0,                      //  If 0, don't select a move. Otherwise select this move slot.
        .dynamax            =   true,                   //  If 1st move select is enabled, dynamax as well.
        .post_raid_delay    =   0 * TICKS_PER_SECOND,   //  Extra wait time after each raid.
    },
    {
        .game_slot          =   1,
        .user_slot          =   3,
        .skips              =   3,
        .backup_save        =   false,
        .always_catchable   =   true,
        .accept_FRs         =   true,
        .move_slot          =   0,
        .dynamax            =   true,
        .post_raid_delay    =   0 * TICKS_PER_SECOND,
    },


    //  DO NOT DELETE THIS
    {0},
};


//  Accept FRs this many games into the future.
//      0   Accept FRs for the game that's currently being hosted.
//      1   Accept FRs for the next game that will be hosted.
//      2   Accept FRs for the game that will be hosted 2 from now.
//      3   ...
const uint8_t FR_FORWARD_ACCEPT =   1;


//  Rollover Prevention
//
//  This option is only relevant if all dens being hosted have (skips = 0).
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


