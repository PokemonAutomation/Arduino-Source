/*  Egg Feedback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggFeedback_H
#define PokemonAutomation_PokemonBDSP_EggFeedback_H

#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
    class BotBaseContext;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{


void hatch_egg(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
void hatch_party(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, size_t eggs = 5);

void withdraw_1st_column_from_overworld(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);


void release(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);



}
}
}
#endif
