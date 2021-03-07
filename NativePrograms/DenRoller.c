/*  Pokemon Sword & Shield: Den Roller
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "PokemonSwShPrograms/DenRoller.h"



//  # of frames to roll.
const uint8_t SKIPS                 =   3;

//  ALWAYS_CATCHABLE    All pokemon in this den are catchable.
//  MAYBE_UNCATCHABLE   There may be uncatchable pokemon.
const Catchability CATCHABILITY     =   ALWAYS_CATCHABLE;

//  View the rolled pokemon for this long before resetting.
const uint16_t VIEW_TIME            =   5 * TICKS_PER_SECOND;


