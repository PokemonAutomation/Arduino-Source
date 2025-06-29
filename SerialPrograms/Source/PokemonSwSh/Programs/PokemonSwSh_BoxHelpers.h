/*  Box Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxHelpers_H
#define PokemonAutomation_PokemonSwSh_BoxHelpers_H

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


static inline void box_scroll(ProControllerContext& context, DpadPosition direction){
    ssf_press_dpad_ptv(context, direction, 200ms, 104ms);
}



}
}
}
#endif
