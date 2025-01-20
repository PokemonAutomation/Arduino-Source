/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggFeedback_H
#define PokemonAutomation_PokemonBDSP_EggFeedback_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void hatch_egg(VideoStream& stream, SwitchControllerContext& context);
void hatch_party(VideoStream& stream, SwitchControllerContext& context, size_t eggs = 5);

void withdraw_1st_column_from_overworld(VideoStream& stream, SwitchControllerContext& context);


void release(VideoStream& stream, SwitchControllerContext& context);



}
}
}
#endif
