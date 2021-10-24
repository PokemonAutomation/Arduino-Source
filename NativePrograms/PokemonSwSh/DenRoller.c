/*  Pokemon Sword & Shield: Den Roller
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Programs/DenRoller.h"



//  # of frames to roll.
const uint8_t SKIPS                 =   3;

//  ALWAYS_CATCHABLE    All pokemon in this den are catchable.
//  MAYBE_UNCATCHABLE   There may be uncatchable pokemon.
const Catchability CATCHABILITY     =   ALWAYS_CATCHABLE;

//  View the rolled pokemon for this long before resetting.
const uint16_t VIEW_TIME            =   5 * TICKS_PER_SECOND;


