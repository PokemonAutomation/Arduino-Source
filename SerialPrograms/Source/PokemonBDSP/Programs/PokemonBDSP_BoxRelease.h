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
void detach(SwitchControllerContext& context);
void detach_box(SwitchControllerContext& context, uint16_t box_scroll_delay);

//  Release Pokemon.
void release(SwitchControllerContext& context);
void release_box(SwitchControllerContext& context, Milliseconds box_scroll_delay);
void release_boxes(
    SwitchControllerContext& context,
    uint8_t boxes,
    Milliseconds box_scroll_delay,
    Milliseconds box_change_delay
);


}
}
}
#endif
