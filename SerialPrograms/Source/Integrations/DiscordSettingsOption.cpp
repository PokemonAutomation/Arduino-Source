/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/Options/BatchWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "DiscordSettingsOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Integration{





DiscordMessageSettingsOption::DiscordMessageSettingsOption()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , instance_name(
        false,
        "<b>Instance Name:</b><br>If you are running multiple instances of this program, give it a name to distinguish them in notifications.",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "(e.g. Living Room Switch)"
    )
    , user_id(
        false,
        "<b>Discord User ID:</b><br>Set this to your discord user ID to receive pings. Your ID is a number.",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "123456789012345678"
    )
    , message(
        false,
        "<b>Discord Message:</b><br>Message to put on every discord notification.",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "(e.g. Kim's Shiny Hunt)"
    )
{
    PA_ADD_OPTION(instance_name);
    PA_ADD_OPTION(user_id);
//    if (PreloadSettings::instance().DEVELOPER_MODE){
//        PA_ADD_OPTION(message);
//    }
}
class DiscordMessageSettingsOptionUI : public BatchWidget{
public:
    DiscordMessageSettingsOptionUI(QWidget& parent, DiscordMessageSettingsOption& value);
};
DiscordMessageSettingsOptionUI::DiscordMessageSettingsOptionUI(QWidget& parent, DiscordMessageSettingsOption& value)
    : BatchWidget(parent, value)
{}
ConfigWidget* DiscordMessageSettingsOption::make_QtWidget(QWidget& parent){
    return new DiscordMessageSettingsOptionUI(parent, *this);
}






DiscordSettingsOption::DiscordSettingsOption()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
{
    PA_ADD_OPTION(message);
    PA_ADD_OPTION(webhooks);
#ifdef PA_DPP
    PA_ADD_OPTION(integration);
#endif
}





}
}

