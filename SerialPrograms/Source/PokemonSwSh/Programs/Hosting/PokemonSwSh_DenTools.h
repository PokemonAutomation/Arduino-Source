/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenTools_H
#define PokemonAutomation_PokemonSwSh_DenTools_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(
    SwitchControllerContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    bool watts,
    bool online
);
void enter_lobby(
    SwitchControllerContext& context,
    Milliseconds OPEN_ONLINE_DEN_LOBBY_DELAY,
    bool online,
    Catchability catchability
);

void roll_den(
    VideoStream& stream, SwitchControllerContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    Milliseconds OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
);
void rollback_date_from_home(SwitchControllerContext& context, uint8_t skips);





}
}
}
#endif
