/*  Box Release Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxReleaseTools_H
#define PokemonAutomation_PokemonBDSP_BoxReleaseTools_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

//  Detach items.
void detach(const BotBaseContext& context);
void detach_box(const BotBaseContext& context, uint16_t box_scroll_delay);

//  Release Pokemon.
void release(const BotBaseContext& context);
void release_box(const BotBaseContext& context, uint16_t box_scroll_delay);
void release_boxes(
    const BotBaseContext& context,
    uint8_t boxes,
    uint16_t box_scroll_delay,
    uint16_t box_change_delay
);


}
}
}
#endif
