/*  Box Release Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxReleaseTools_H
#define PokemonAutomation_PokemonBDSP_BoxReleaseTools_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

//  Detach items.
void detach(ControllerContext& context);
void detach_box(ControllerContext& context, uint16_t box_scroll_delay);

//  Release Pokemon.
void release(ControllerContext& context);
void release_box(ControllerContext& context, uint16_t box_scroll_delay);
void release_boxes(
    ControllerContext& context,
    uint8_t boxes,
    uint16_t box_scroll_delay,
    uint16_t box_change_delay
);


}
}
}
#endif
