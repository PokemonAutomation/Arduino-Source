/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenTools_H
#define PokemonAutomation_PokemonSwSh_DenTools_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(
    ProControllerContext& context,
    Milliseconds ENTER_ONLINE_DEN_DELAY,
    bool watts,
    bool online
);
void enter_lobby(
    ProControllerContext& context,
    Milliseconds OPEN_ONLINE_DEN_LOBBY_DELAY,
    bool online,
    Catchability catchability
);

void roll_den(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds ENTER_ONLINE_DEN_DELAY,
    Milliseconds OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
);
void rollback_date_from_home(ProControllerContext& context, uint8_t skips);





}
}
}
#endif
