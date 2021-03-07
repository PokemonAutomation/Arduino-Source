/*  Settings
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 * 
 *  These are global setting shared by all programs.
 * 
 *  You shouldn't need to change any of the timing settings unless you are
 *  encountering problems.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PokemonSettings.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Egg Options

//  true    =   Send Pokemon to boxes is "Automatic".
//  false   =   Send Pokemon to boxes is "Manual".
bool AUTO_DEPOSIT               =   true;

//  The daycare lady has an extra line of text in Japanese. Set this to true if
//  you are running any of the egg programs in a Japanese game.
bool EGG_FETCH_EXTRA_LINE       =   false;

//  Time needed to mash B to fetch an egg and return to overworld when
//  auto-deposit is off.
uint16_t FETCH_EGG_MASH_DELAY   =   800;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Den Options

//  Which method to use to bypass the uncatchable Pokemon prompt?
//      true    =   Use a fast (but potentially unreliable) method.
//      false   =   Use a slower (by about 5 seconds) method.
bool DODGE_UNCATCHABLE_PROMPT_FAST      =   false;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Options
//
//  You really shouldn't need to touch anything from here on down unless you are
//  encountering problems and are trying to debug it yourself.
//

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  General Timings

//  Time to accept FRs before returning to den lobby.
uint16_t AUTO_FR_DURATION                   =   8 * TICKS_PER_SECOND;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Menu Navigation Timings

//  Delay to bring up the menu when pressing X in the overworld.
uint16_t OVERWORLD_TO_MENU_DELAY            =   120;

//  Delay to go from menu back to overworld.
uint16_t MENU_TO_OVERWORLD_DELAY            =   250;

//  Delay from pressing home to entering the the Switch home menu.
//  This affects the speed of date-spamming programs (i.e. watt farmer)
uint16_t GAME_TO_HOME_DELAY_FAST            =   100;
uint16_t GAME_TO_HOME_DELAY_SAFE            =   125;


//  Delay to enter game from home menu.
uint16_t HOME_TO_GAME_DELAY                 =   3 * TICKS_PER_SECOND;

//  Time needed to open Y-COMM.
uint16_t OPEN_YCOMM_DELAY                   =   200;

//  Delay to enter your Switch profile.
uint16_t ENTER_PROFILE_DELAY                =   2 * TICKS_PER_SECOND;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Start Game Timings

//  If starting the game requires checking the internet, wait this long for it.
uint16_t START_GAME_INTERNET_CHECK_DELAY    =   3 * TICKS_PER_SECOND;

//  Delays to start and enter the game when it isn't running.
uint16_t START_GAME_MASH                    =   2 * TICKS_PER_SECOND;   //  1. Mash A for this long to start the game.
uint16_t START_GAME_WAIT                    =   20 * TICKS_PER_SECOND;  //  2. Wait this long for the game to load.
uint16_t ENTER_GAME_MASH                    =   3 * TICKS_PER_SECOND;   //  3. Mash A for this long to enter the game.
uint16_t ENTER_GAME_WAIT                    =   11 * TICKS_PER_SECOND;  //  4. Wait this long for the game to enter the overworld.


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Den Timings

//  Time needed to enter a den on final button press.
uint16_t ENTER_OFFLINE_DEN_DELAY        =   125;

//  Time from exiting den after a day-skip to when you can collect watts and re-enter it.
uint16_t REENTER_DEN_DELAY              =   5 * TICKS_PER_SECOND;

//  Time needed to collect watts.
uint16_t COLLECT_WATTS_OFFLINE_DELAY    =   80;
uint16_t COLLECT_WATTS_ONLINE_DELAY     =   3 * TICKS_PER_SECOND;

//  Time needed to bypass uncatchable pokemon prompt.
uint16_t UNCATCHABLE_PROMPT_DELAY       =   110;

//  Time needed to open a den lobby on local connection.
uint16_t OPEN_LOCAL_DEN_LOBBY_DELAY     =   3 * TICKS_PER_SECOND;

//  Time needed to enter Switch Pokemon.
uint16_t ENTER_SWITCH_POKEMON           =   4 * TICKS_PER_SECOND;

//  Time needed to exit Switch Pokemon back to den lobby.
uint16_t EXIT_SWITCH_POKEMON            =   3 * TICKS_PER_SECOND;

//  Always 3 minutes.
uint16_t FULL_LOBBY_TIMER               =   180 * TICKS_PER_SECOND;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Box Timings (for egg programs)

uint16_t BOX_SCROLL_DELAY           =    20;     //  Delay to move the cursor.
uint16_t BOX_CHANGE_DELAY           =    80;     //  Delay to change boxes.
uint16_t BOX_PICKUP_DROP_DELAY      =    90;     //  Delay to pickup/drop pokemon.

uint16_t MENU_TO_POKEMON_DELAY      =    300;    //  Delay to enter Pokemon menu.
uint16_t POKEMON_TO_BOX_DELAY       =    300;    //  Delay to enter box system.
uint16_t BOX_TO_POKEMON_DELAY       =    250;    //  Delay to exit box system.
uint16_t POKEMON_TO_MENU_DELAY      =    250;    //  Delay to return to menu.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
