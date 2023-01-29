/*  Auto Host Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSV/Options/PokemonSV_AutoHostOptions.h"
#include "PokemonSV_AutoHostTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void send_host_announcement(
    ProgramEnvironment& env, ConsoleHandle& host_console,
    const std::string& lobby_code, bool show_raid_code,
    const std::string& description,
    EventNotificationOption& NOTIFICATION_RAID_POST
){
    std::vector<std::pair<std::string, std::string>> messages;
    if (!description.empty()){
        messages.emplace_back("Description:", std::move(description));
    }

    VideoSnapshot snapshot = host_console.video().snapshot();
    if (show_raid_code && !lobby_code.empty()){
        messages.emplace_back("Raid Code:", lobby_code);
    }

    send_program_notification(
        env, NOTIFICATION_RAID_POST,
        Color(0),
        "Tera Raid Notification",
        messages, "",
        snapshot
    );
}



}
}
}
