/*  General Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Routines_H
#define PokemonAutomation_NintendoSwitch_Commands_Routines_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game(VideoStream& stream, ProControllerContext& device);




}
}
#endif
