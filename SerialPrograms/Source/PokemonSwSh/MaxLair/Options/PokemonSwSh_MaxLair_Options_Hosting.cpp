/*  Max Lair Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

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
        4, ""
    )
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay:</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        "60 * TICKS_PER_SECOND"
    )
    , CONNECT_TO_INTERNET_DELAY(
        "<b>Connect to Internet Delay:</b><br>Time from \"Connect to Internet\" to when you're ready to start adventure.",
        "20 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS("Live-Hosting Announcements", true)
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(CONNECT_TO_INTERNET_DELAY);
    PA_ADD_OPTION(NOTIFICATIONS);
}
QString HostingSettings::check_validity(size_t consoles) const{
    if (consoles != 1 && !RAID_CODE.code_enabled()){
        return "You must use a code when running with multiple Switches.";
    }
    return QString();
}
ConfigOptionUI* HostingSettings::make_ui(QWidget& parent){
    return new HostingSettingsUI(parent, *this);
}

HostingSettingsUI::HostingSettingsUI(QWidget& parent, HostingSettings& value)
    : GroupOptionUI(parent, value)
{
    update_option_visibility();

    EnumDropdownOptionUI* mode = nullptr;
    for (ConfigOptionUI* option : m_options){
        if (&option->option() == &value.MODE){
            mode = static_cast<EnumDropdownOptionUI*>(option);
        }
    }
    connect(
        mode, &EnumDropdownOptionUI::on_changed,
        this, [=]{
            update_option_visibility();
            this->update_visibility();
        }
    );
}
void HostingSettingsUI::update_option_visibility(){
    HostingSettings& value = static_cast<HostingSettings&>(m_value);
    if ((size_t)value.MODE == (size_t)HostingMode::HOST_ONLINE){
        value.CONNECT_TO_INTERNET_DELAY.visibility = ConfigOptionState::ENABLED;
    }else{
        value.CONNECT_TO_INTERNET_DELAY.visibility = ConfigOptionState::DISABLED;
    }
    if ((size_t)value.MODE != (size_t)HostingMode::NOT_HOSTING){
        value.NOTIFICATIONS.visibility = ConfigOptionState::ENABLED;
    }else{
        value.NOTIFICATIONS.visibility = ConfigOptionState::DISABLED;
    }
}



}
}
}
}
