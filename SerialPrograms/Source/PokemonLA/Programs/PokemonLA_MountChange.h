/*  Mount Change
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_MountChange_H
#define PokemonAutomation_PokemonLA_MountChange_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


//  Be careful when changing away from Braviary (on) since you will fall down.
void change_mount(ConsoleHandle& console, BotBaseContext& context, MountState mount);

//  Dismount player character.
//  Be careful when changing away from Braviary (on) since you will fall down.
void dismount(ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif
