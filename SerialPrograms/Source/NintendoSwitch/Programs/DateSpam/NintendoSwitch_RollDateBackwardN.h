/*  Nintendo Switch Roll Date Backward N
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RollDateBackwardN_H
#define PokemonAutomation_NintendoSwitch_RollDateBackwardN_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void roll_date_backward_N(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t skips, bool fast
);





}
}
#endif
