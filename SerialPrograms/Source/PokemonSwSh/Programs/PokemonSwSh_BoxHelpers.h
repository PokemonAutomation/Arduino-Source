/*  Box Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxHelpers_H
#define PokemonAutomation_PokemonSwSh_BoxHelpers_H

#include "CommonFramework/Language.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"

namespace PokemonAutomation{
class VideoStream;
namespace NintendoSwitch{
namespace PokemonSwSh{


static inline void box_scroll(ProControllerContext& context, DpadPosition direction){
    ssf_press_dpad_ptv(context, direction, 200ms, 104ms);
}

//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  try to change the view to the judge. However, it may land on the stats instead.
//  If it can't land on either stats or judge and `throw_exception` is true, it will throw an exception.
//  Return true if it successfully changed view to judge (or stats if judge is not found)
bool change_view_to_stats_or_judge(
    VideoStream& stream, ProControllerContext& context,
    bool throw_exception = true
);


//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  change the view to the judge. If it fails, it will OperationFailedException::fire.
void change_view_to_judge(
    VideoStream& stream, ProControllerContext& context,
    Language language
);




}
}
}
#endif
