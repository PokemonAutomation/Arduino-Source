/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NativePrograms/PokemonSwSh/Libraries/PokemonSettings.h"


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
extern const uint8_t SKIPS;                 //  Set to 0 if hard-locked on desired frame.
extern const bool HOST_ONLINE;              //  true = host online, false = host offline
extern const uint16_t LOBBY_WAIT_DELAY;     //  3:00 - 60 secs = 2:00


//  Backup Save: (for soft-lock hosting)
//      If set to true, the game will load backup save upon soft-reset.
//  The idea here is to roll forward from the save frame to the shiny frame.
//  Once on the shiny frame (with the desired softlock), you enter and exit the
//  camp to setup a backup save on the shiny frame. That way, you can load the
//  backup save on the shiny frame while leaving the real save safely 3 or more
//  frames before.
//
//  This option only makes sense when SKIPS = 0.
extern const bool BACKUP_SAVE;


//  ALWAYS_CATCHABLE    All pokemon in this den are catchable.
//  MAYBE_UNCATCHABLE   There may be uncatchable pokemon.
extern const Catchability CATCHABILITY;


//  Friend-Request Accepting:
//      0   =   Disabled. Do not automatically accept friend requests.
//      #   =   Automatically accept friend requests for the N'th user.
//
//  This number is the slot position of the user at the top of the Home menu.
//  The left-most user is slot 1. This option only works if "HOST_ONLINE" is true.
//
//  Friend accepting is done while waiting in the raid lobby. Therefore it does
//  not slow down the program.
extern const uint8_t FRIEND_ACCEPT_USER_SLOT;


//  Additional Raid Delay:
//      If you are farming a pokemon and the time between raids is too short to
//  join consecutive raids, use this to add time between raids. This extra wait
//  time is done after the code is entered, but before opening the lobby.
extern const uint16_t EXTRA_DELAY_BETWEEN_RAIDS;


//  Select 1st Move:
//      MOVE_SLOT   =   0   Do not select a move for first turn.
//      MOVE_SLOT   =   #   Select this move for the first turn.
extern const uint8_t MOVE_SLOT;
extern const bool DYNAMAX;          //  Must set to false if you cannot dmax.
extern const uint8_t TROLL_HOSTING; //  0 disables the troll hosting option, 1 attacks the first ally, 2 attacks the second one, 3 attacks the third one. Dynamaxing will disable this option.


//  Alternate Games: (experimental feature)
//      If set to true, alternate between games 1 and 2.
//  Basically if you have both Sword and Shield on the same user and you wish
//  to host dens on both with the same settings.
extern const bool ALTERNATE_GAMES;


//  Rollover Prevention
//
//  This option is only relevant if (SKIPS = 0).
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


