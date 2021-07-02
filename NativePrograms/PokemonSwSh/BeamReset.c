/*  Pokemon Sword & Shield: Beam Resetter
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      Please refer to the user manual for instructions and documentation.
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "Programs/BeamReset.h"



//  Wait in the Home menu for this long before resetting the game.
const uint16_t DELAY_BEFORE_RESET   =   5 * TICKS_PER_SECOND;

//  Talking to the den has an extra line in German. Set this to true if this
//  is the case.
const bool EXTRA_LINE   =   false;


