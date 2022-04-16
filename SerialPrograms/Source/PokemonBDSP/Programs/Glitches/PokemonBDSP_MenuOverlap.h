/*  Menu Overlap Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MenuOverlap_H
#define PokemonAutomation_PokemonBDSP_MenuOverlap_H

#include "CommonFramework/Tools/ConsoleHandle.h"

class QImage;

namespace PokemonAutomation{
    class BotBaseContext;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Activate the menu overlap glitch from the overworld and back out.
//  Returns the image of the overworld prior to entering the menus.
//  Returns null image if the operation failed.
QImage activate_menu_overlap_from_overworld(ConsoleHandle& console, BotBaseContext& context);


//  Press B to back out all the way to the overworld with the menu on overlapped on top.
bool back_out_to_overworld_with_overlap(
    ConsoleHandle& console, BotBaseContext& context,
    const QImage& start,    //  Image of the overworld prior to enter the menus.
    uint16_t mash_B_start   //  Mash B for this long before using feedback.
);

//  Mash B to back out all the overworld completely. (disabling glitch)
void back_out_to_overworld(
    ConsoleHandle& console, BotBaseContext& context,
    const QImage& start
);


}
}
}
#endif
