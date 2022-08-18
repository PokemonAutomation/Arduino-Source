/*  Max Lair Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Common/Qt/Options/DropdownWidget.h"
#include "PokemonSwSh_MaxLair_Options_Hosting.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



HostingSettings::HostingSettings()
    : GroupOption("Hosting Options")
    , MODE(
        "<b>Mode:</b>",
        {
            "Not Hosting: Run by yourself using only Switches controlled by this program.",
            "Host Locally: Allow other local Switches to join.",
            "Host Online: Allow other people online to join.",
        }
        , 0
    )
    , RAID_CODE(
        "<b>Raid Code:</b><br>Required if using multiple Switches. "
        "Random code is strongly recommended when hosting to ensure your own Switches get in.",
        8, 4, ""
    )
    , CONNECT_TO_INTERNET_DELAY(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to start adventure.",
        TICKS_PER_SECOND,
        "20 * TICKS_PER_SECOND"
    )
    , START_DELAY(
        "<b>Start Delay:</b><br>Wait this long before entering the lobby.<br><br>"
        "If two Switches open a lobby at the same time, they might not see each other and "
        "thus fail to join each other. If you are joining someone else's auto-host, you "
        "will want to set this to 3 seconds or more to make sure that the host opens the "
        "lobby before everyone else tries to join.",
        TICKS_PER_SECOND,
        "0 * TICKS_PER_SECOND"
    )
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        TICKS_PER_SECOND,
        "60 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS("Live-Hosting Announcements", true)
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY);
    PA_ADD_OPTION(START_DELAY);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(NOTIFICATIONS);
}
std::string HostingSettings::check_validity(size_t consoles) const{
    if (consoles != 1 && !RAID_CODE.code_enabled()){
        return "You must use a code when running with multiple Switches.";
    }
    return std::string();
}
ConfigWidget* HostingSettings::make_ui(QWidget& parent){
    return new HostingSettingsUI(parent, *this);
}

HostingSettingsUI::~HostingSettingsUI(){
    m_value.MODE.remove_listener(*this);
}
HostingSettingsUI::HostingSettingsUI(QWidget& parent, HostingSettings& value)
    : GroupWidget(parent, value)
    , m_value(value)
{
    update_option_visibility();
    m_value.MODE.add_listener(*this);
}
void HostingSettingsUI::update_option_visibility(){
    HostingSettings& value = static_cast<HostingSettings&>(m_value);
    if ((size_t)value.MODE == (size_t)HostingMode::HOST_ONLINE){
        value.CONNECT_TO_INTERNET_DELAY.set_visibility(ConfigOptionState::ENABLED);
    }else{
        value.CONNECT_TO_INTERNET_DELAY.set_visibility(ConfigOptionState::DISABLED);
    }
    if ((size_t)value.MODE != (size_t)HostingMode::NOT_HOSTING){
        value.START_DELAY.set_visibility(ConfigOptionState::ENABLED);
        value.NOTIFICATIONS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        value.START_DELAY.set_visibility(ConfigOptionState::DISABLED);
        value.NOTIFICATIONS.set_visibility(ConfigOptionState::DISABLED);
    }
}
void HostingSettingsUI::value_changed(){
    QMetaObject::invokeMethod(this, [=]{
        update_option_visibility();
        this->update();
    }, Qt::QueuedConnection);
}



}
}
}
}
