/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Common/PokemonSwSh/PokemonSettings.h"


//  # of frames to roll.
extern const uint8_t SKIPS;

//  ALWAYS_CATCHABLE    All pokemon in this den are catchable.
//  MAYBE_UNCATCHABLE   There may be uncatchable pokemon.
extern const Catchability CATCHABILITY;

//  View the rolled pokemon for this long before resetting.
extern const uint16_t VIEW_TIME;
