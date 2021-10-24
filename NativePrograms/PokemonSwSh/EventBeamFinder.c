/*  Pokemon Sword & Shield: Event Beam Finder
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Programs/EventBeamFinder.h"



//  View the den for this duration before moving on.
const uint16_t WAIT_TIME_IN_DEN = 5 * TICKS_PER_SECOND;


