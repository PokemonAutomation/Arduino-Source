/*  Max Lair Hosting
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_MaxLair_Options_Hosting.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



HostingSettings::~HostingSettings(){
    MODE.remove_listener(*this);
}
HostingSettings::HostingSettings()
    : GroupOption("Hosting Options", LockMode::LOCK_WHILE_RUNNING)
    , MODE(
        "<b>Mode:</b>",
        {
            {HostingMode::NOT_HOSTING, "none", "Not Hosting: Run by yourself using only Switches controlled by this program."},
            {HostingMode::HOST_LOCALLY, "local", "Host Locally: Allow other local Switches to join."},
            {HostingMode::HOST_ONLINE, "online", "Host Online: Allow other people online to join."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HostingMode::NOT_HOSTING
    )
    , RAID_CODE(
        "<b>Raid Code:</b><br>Required if using multiple Switches. "
        "Random code is strongly recommended when hosting to ensure your own Switches get in.",
        8, 4, ""
    )
    , CONNECT_TO_INTERNET_DELAY0(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to start adventure.",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
    , START_DELAY0(
        "<b>Start Delay:</b><br>Wait this long before entering the lobby.<br><br>"
        "If two Switches open a lobby at the same time, they might not see each other and "
        "thus fail to join each other. If you are joining someone else's auto-host, you "
        "will want to set this to 3 seconds or more to make sure that the host opens the "
        "lobby before everyone else tries to join.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 s"
    )
    , LOBBY_WAIT_DELAY0(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
    , NOTIFICATIONS("Live-Hosting Announcements", true)
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY0);
    PA_ADD_OPTION(START_DELAY0);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY0);
    PA_ADD_OPTION(NOTIFICATIONS);

    MODE.add_listener(*this);
}
std::string HostingSettings::check_validity(size_t consoles) const{
    if (consoles != 1 && !RAID_CODE.code_enabled()){
        return "You must use a code when running with multiple Switches.";
    }
    return std::string();
}
void HostingSettings::on_config_value_changed(void* object){
    HostingMode mode = MODE;
    if (mode == HostingMode::HOST_ONLINE){
        CONNECT_TO_INTERNET_DELAY0.set_visibility(ConfigOptionState::ENABLED);
    }else{
        CONNECT_TO_INTERNET_DELAY0.set_visibility(ConfigOptionState::DISABLED);
    }
    if (mode != HostingMode::NOT_HOSTING){
        START_DELAY0.set_visibility(ConfigOptionState::ENABLED);
        NOTIFICATIONS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        START_DELAY0.set_visibility(ConfigOptionState::DISABLED);
        NOTIFICATIONS.set_visibility(ConfigOptionState::DISABLED);
    }
}



}
}
}
}
