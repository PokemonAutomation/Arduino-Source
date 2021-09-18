/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenTools_H
#define PokemonAutomation_PokemonSwSh_DenTools_H

#include "Common/PokemonSwSh/PokemonSettings.h"
//#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(const BotBaseContext& context, uint16_t ENTER_ONLINE_DEN_DELAY, bool watts, bool online);
void enter_lobby(const BotBaseContext& context, uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY, bool online, Catchability catchability);

void roll_den(
    const BotBaseContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
);
void rollback_date_from_home(const BotBaseContext& context, uint8_t skips);

void send_raid_notification(
    const QString& program_name,
    ConsoleHandle& console,
    const AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const DenMonReader& reader,
    const StatsTracker& stats_tracker
);




}
}
}
#endif
