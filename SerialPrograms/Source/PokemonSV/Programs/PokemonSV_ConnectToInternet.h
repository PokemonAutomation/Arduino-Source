/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ConnectToInternet_H
#define PokemonAutomation_PokemonSV_ConnectToInternet_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  Connect to internet from the main menu. Stay in the main menu.
void connect_to_internet_from_menu(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  Connect to internet from the overworld. Return to the overworld.
void connect_to_internet_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);



}
}
}
#endif
