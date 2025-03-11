/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include <avr/pgmspace.h>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


//  Raid Code
//      No code:            RANDOM_DIGITS = 0       RAID_CODE = ""
//      Fixed code:         RANDOM_DIGITS = 0       RAID_CODE = "12345678"
//      Random Code:        RANDOM_DIGITS = 4       RAID_CODE = "12345678"
//
//  "RANDOM_DIGITS" specifies how many of the first digits are randomized.
//  If it is less than 8, the remaining digits will be copies of the last digit.
//  This makes it easier to enter the code.
//
//  When using random codes, "RAID_CODE" is used to generate the starting seed.
extern const uint8_t RANDOM_DIGITS;
extern const char* RAID_CODE;


//  General Settings
extern const bool HOST_ONLINE;              //  true = host online, false = host offline
extern const uint16_t LOBBY_WAIT_DELAY;     //  3:00 - 60 secs = 2:00


//  Game List
#if 0
typedef struct{
    //  This is what the struct is supposed to look like. (9 bytes)

    uint8_t game_slot;          //  Game slot at start of program. (must be 1 or 2)
    uint8_t user_slot;          //  User # to host from.

    bool backup_save;           //  Load backup save instead of normal reset.
    bool always_catchable;      //  Pokemon in this den are always catchable.

    bool accept_FRs;            //  Pokemon in this den are always catchable.

    uint8_t skips;              //  # of frame skips to roll before hosting.
    bool accept_FRs;            //  Accept friend requests.

    uint8_t move_slot;          //  If 0, don't select a move. Otherwise select this move slot.
    bool dynamax;               //  If 1st move select is enabled, dynamax as well.

    uint16_t post_raid_delay;   //  Extra wait time after each raid.
} RollingAutoHostSlot;
#else
typedef struct{
    //  Here's the compressed version. (4 bytes)

    uint8_t game_slot     : 2;      //  Game slot at start of program. (must be 1 or 2)
    uint8_t user_slot     : 4;      //  User # to host from.

    bool backup_save      : 1;      //  Load backup save instead of normal reset.
    bool always_catchable : 1;      //  Pokemon in this den are always catchable.

    uint8_t skips         : 3;      //  # of frame skips to roll before hosting.
    bool accept_FRs       : 1;      //  Accept friend requests.

    uint8_t move_slot     : 3;      //  If 0, don't select a move. Otherwise select this move slot.
    bool dynamax          : 1;      //  If 1st move select is enabled, dynamax as well.

    uint16_t post_raid_delay;       //  Extra wait time after each raid.
} RollingAutoHostSlot;
#endif
extern const RollingAutoHostSlot PROGMEM GAME_LIST[];


//  Accept FRs this many games into the future.
//      0   Accept FRs for the game that's currently being hosted.
//      1   Accept FRs for the next game that will be hosted.
//      2   Accept FRs for the game that will be hosted 2 from now.
//      3   ...
extern const uint8_t FR_FORWARD_ACCEPT;


//  Rollover Prevention
//
//  This option is only relevant if all dens being hosted have (skips = 0).
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent the den from rolling over. This feature requres time-sync be disabled.
extern const uint32_t TOUCH_DATE_INTERVAL;



//  Increase these if your internet is slow.
extern const uint16_t CONNECT_TO_INTERNET_DELAY;    //  Time from "Connect to Internet" to when you're ready to enter den.
extern const uint16_t ENTER_ONLINE_DEN_DELAY;       //  "Communicating" when entering den while online.
extern const uint16_t START_RAID_DELAY;             //  Delay from "Invite Others" to when the clock starts ticking.

extern const uint16_t RAID_START_TO_EXIT_DELAY;     //  Time from start raid to reset. (when not selecting move)
extern const uint16_t DELAY_TO_SELECT_MOVE;         //  This + RAID_START_TO_EXIT_DELAY = time from start raid to select move.


