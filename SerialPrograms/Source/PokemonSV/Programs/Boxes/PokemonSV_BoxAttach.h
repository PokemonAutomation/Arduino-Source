/*  Box Attach
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxAttach_H
#define PokemonAutomation_PokemonSV_BoxAttach_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  With the cursor over the item you want to attach, attach to the current
//  Pokemon, replacing if neccessary.
void attach_item_from_bag(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    size_t& errors
);

//  With the cursor over the pokemon you want to attach the item to.
//  Make sure you are in the item view.
void attach_item_from_box(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    size_t category_index,
    size_t& errors
);



}
}
}
#endif
