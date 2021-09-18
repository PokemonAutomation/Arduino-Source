/*  Auto-Host Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


AutoHostNotificationOption::AutoHostNotificationOption(QString label)
    : GroupOption(std::move(label))
    , ENABLE("<b>Enable Discord Notifications:</b>", false)
    , DESCRIPTION("<b>Description:</b> What you're hosting, your IGN, friend code(s), etc...", "")
    , SHOW_STATS("<b>Show Stats:</b> Show program stats for this session.", true)
    , SCREENSHOT("<b>Notification Screenshot:</b><br>Attach screenshot of the den to notifications.")
{
    PA_ADD_OPTION(ENABLE);
    PA_ADD_OPTION(DESCRIPTION);
    PA_ADD_OPTION(SHOW_STATS);
    PA_ADD_OPTION(SCREENSHOT);
}



}
}
}
