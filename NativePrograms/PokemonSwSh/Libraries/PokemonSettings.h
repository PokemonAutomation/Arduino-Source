/*  Settings
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_Settings_H
#define PokemonAutomation_Settings_H

#include <stdbool.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Egg Options

//  true    =   Send Pokemon to boxes is "Automatic".
//  false   =   Send Pokemon to boxes is "Manual".
extern bool AUTO_DEPOSIT;

//  The daycare lady has an extra line of text in Japanese. Set this to true if
//  you are running any of the egg programs in a Japanese game.
extern bool EGG_FETCH_EXTRA_LINE;

//  Time needed to mash B to fetch an egg and return to overworld when
//  auto-deposit is off.
extern uint16_t FETCH_EGG_MASH_DELAY;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Den Options

typedef enum{
    ALWAYS_CATCHABLE,
    MAYBE_UNCATCHABLE,
    ALWAYS_UNCATCHABLE,
} Catchability;

//  Which method to use to bypass the uncatchable Pokemon prompt?
//      true    =   Use a fast (but potentially unreliable) method.
//      false   =   Use a slower (by about 5 seconds) method.
extern bool DODGE_UNCATCHABLE_PROMPT_FAST;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Options

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  General Timings

//  Time to accept FRs before returning to den lobby.
extern uint16_t AUTO_FR_DURATION;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Menu Navigation Timings

//  Delay to bring up the menu when pressing X in the overworld.
extern uint16_t OVERWORLD_TO_MENU_DELAY;

//  Delay to go from menu back to overworld.
extern uint16_t MENU_TO_OVERWORLD_DELAY;

//  Delay from pressing home to entering the the Switch home menu.
//  This affects the speed of date-spamming programs (i.e. watt farmer)
extern uint16_t GAME_TO_HOME_DELAY_FAST;
extern uint16_t GAME_TO_HOME_DELAY_SAFE;

//  Delay to enter game from home menu.
extern uint16_t HOME_TO_GAME_DELAY;

//  Time needed to open Y-COMM.
extern uint16_t OPEN_YCOMM_DELAY;

//  Delay to enter your Switch profile.
extern uint16_t ENTER_PROFILE_DELAY;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Start Game Timings

//  Delays to start and enter the game when it isn't running.
extern uint16_t START_GAME_MASH;  //  1. Mash A for this long to start the game.
extern uint16_t START_GAME_WAIT;  //  2. Wait this long for the game to load.
extern uint16_t ENTER_GAME_MASH;  //  3. Mash A for this long to enter the game.
extern uint16_t ENTER_GAME_WAIT;  //  4. Wait this long for the game to fully load into the overworld.


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Den Timings

//  Time needed to enter a den on final button press.
extern uint16_t ENTER_OFFLINE_DEN_DELAY;

//  Time from exiting den after a day-skip to when you can collect watts and re-enter it.
extern uint16_t REENTER_DEN_DELAY;

//  Time needed to collect watts.
extern uint16_t COLLECT_WATTS_OFFLINE_DELAY;
extern uint16_t COLLECT_WATTS_ONLINE_DELAY;

//  Time needed to bypass uncatchable pokemon prompt.
extern uint16_t UNCATCHABLE_PROMPT_DELAY;

//  Time needed to open a den lobby on local connection.
extern uint16_t OPEN_LOCAL_DEN_LOBBY_DELAY;

//  Time needed to enter Switch Pokemon.
extern uint16_t ENTER_SWITCH_POKEMON;

//  Time needed to exit Switch Pokemon back to den lobby.
extern uint16_t EXIT_SWITCH_POKEMON;

//  Always 3 minutes.
extern uint16_t FULL_LOBBY_TIMER;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Box Timings (for egg programs)

extern uint16_t BOX_SCROLL_DELAY;         //  Delay to move the cursor.
extern uint16_t BOX_CHANGE_DELAY;         //  Delay to change boxes.
extern uint16_t BOX_PICKUP_DROP_DELAY;    //  Delay to pickup/drop pokemon.

extern uint16_t MENU_TO_POKEMON_DELAY;    //  Delay to enter Pokemon menu.
extern uint16_t POKEMON_TO_BOX_DELAY;     //  Delay to enter box system.
extern uint16_t BOX_TO_POKEMON_DELAY;     //  Delay to exit box system.
extern uint16_t POKEMON_TO_MENU_DELAY;    //  Delay to return to menu.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
