/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenTools_H
#define PokemonAutomation_PokemonSwSh_DenTools_H

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(BotBaseContext& context, uint16_t ENTER_ONLINE_DEN_DELAY, bool watts, bool online);
void enter_lobby(BotBaseContext& context, uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY, bool online, Catchability catchability);

void roll_den(
    BotBaseContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
);
void rollback_date_from_home(BotBaseContext& context, uint8_t skips);





}
}
}
#endif
