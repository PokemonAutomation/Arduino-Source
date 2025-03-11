/*  Box Release Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxReleaseTools_H
#define PokemonAutomation_PokemonBDSP_BoxReleaseTools_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

//  Detach items.
void detach(ProControllerContext& context);
void detach_box(ProControllerContext& context, uint16_t box_scroll_delay);

//  Release Pokemon.
void release(ProControllerContext& context);
void release_box(ProControllerContext& context, Milliseconds box_scroll_delay);
void release_boxes(
    ProControllerContext& context,
    uint8_t boxes,
    Milliseconds box_scroll_delay,
    Milliseconds box_change_delay
);


}
}
}
#endif
