/*  Settings
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 * 
 *  These are global setting shared by all programs.
 * 
 *  You shouldn't need to change any of the timing settings unless you are
 *  encountering problems.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Libraries/PokemonBDSP_Settings.h"

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
//  Menu Navigation Timings

//  Delay to bring up the menu when pressing X in the overworld.
uint16_t PokemonBDSP_OVERWORLD_TO_MENU_DELAY            =   250;

//  Delay to go from menu back to overworld.
uint16_t PokemonBDSP_MENU_TO_OVERWORLD_DELAY            =   250;

//  Delay from pressing home to entering the the Switch home menu.
uint16_t PokemonBDSP_GAME_TO_HOME_DELAY                 =   125;

//  Delay to enter game from home menu.
uint16_t PokemonBDSP_HOME_TO_GAME_DELAY                 =   3 * TICKS_PER_SECOND;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Start Game Timings

//  Delays to start and enter the game when it isn't running.
uint16_t PokemonBDSP_START_GAME_MASH                    =   2 * TICKS_PER_SECOND;   //  1. Mash A for this long to start the game.
uint16_t PokemonBDSP_START_GAME_WAIT                    =   40 * TICKS_PER_SECOND;  //  2. Wait this long for the game to load.
uint16_t PokemonBDSP_ENTER_GAME_MASH                    =   5 * TICKS_PER_SECOND;   //  3. Mash A for this long to enter the game.
uint16_t PokemonBDSP_ENTER_GAME_WAIT                    =   30 * TICKS_PER_SECOND;  //  4. Wait this long for the game to enter the overworld.


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Box Timings (for egg programs)

uint16_t PokemonBDSP_BOX_SCROLL_DELAY           =    30;     //  Delay to move the cursor.
uint16_t PokemonBDSP_BOX_CHANGE_DELAY           =    200;    //  Delay to change boxes.
uint16_t PokemonBDSP_BOX_PICKUP_DROP_DELAY      =    50;     //  Delay to pickup/drop pokemon.

uint16_t PokemonBDSP_MENU_TO_POKEMON_DELAY      =    300;    //  Delay to enter Pokemon menu.
uint16_t PokemonBDSP_POKEMON_TO_BOX_DELAY       =    300;    //  Delay to enter box system.
uint16_t PokemonBDSP_BOX_TO_POKEMON_DELAY       =    250;    //  Delay to exit box system.
uint16_t PokemonBDSP_POKEMON_TO_MENU_DELAY      =    250;    //  Delay to return to menu.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
