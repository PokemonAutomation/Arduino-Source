/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggFeedback_H
#define PokemonAutomation_PokemonBDSP_EggFeedback_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void hatch_egg(ConsoleHandle& console, ControllerContext& context);
void hatch_party(ConsoleHandle& console, ControllerContext& context, size_t eggs = 5);

void withdraw_1st_column_from_overworld(ConsoleHandle& console, ControllerContext& context);


void release(ConsoleHandle& console, ControllerContext& context);



}
}
}
#endif
