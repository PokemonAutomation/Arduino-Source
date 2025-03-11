/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggFeedback_H
#define PokemonAutomation_PokemonBDSP_EggFeedback_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void hatch_egg(VideoStream& stream, ProControllerContext& context);
void hatch_party(VideoStream& stream, ProControllerContext& context, size_t eggs = 5);

void withdraw_1st_column_from_overworld(VideoStream& stream, ProControllerContext& context);


void release(VideoStream& stream, ProControllerContext& context);



}
}
}
#endif
