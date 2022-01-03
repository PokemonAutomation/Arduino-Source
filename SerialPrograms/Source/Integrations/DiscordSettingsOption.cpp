/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "DiscordSettingsOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Integration{


DiscordMessageSettingsOption::DiscordMessageSettingsOption()
    : instance_name(
        false,
        "<b>Instance Name:</b><br>If you are running multiple instances of this program, give it a name to distinguish them in notifications.",
        "",
        "(e.g. Living Room Switch)"
    )
    , user_id(
        false,
        "<b>Discord User ID:</b><br>Set this to your discord user ID to receive pings. Your ID is a number.",
        "",
        "123456789012345678"
    )
    , message(
        false,
        "<b>Discord Message:</b><br>Message to put on every discord notification.",
        "",
        "(e.g. Kim's Shiny Hunt)"
    )
{
    PA_ADD_OPTION(instance_name);
    PA_ADD_OPTION(user_id);
    PA_ADD_OPTION(message);
}
DiscordMessageSettingsOptionUI::DiscordMessageSettingsOptionUI(QWidget& parent, DiscordMessageSettingsOption& value)
    : BatchWidget(parent, value)
{}






DiscordSettingsOption::DiscordSettingsOption(){
    PA_ADD_OPTION(message);
    PA_ADD_OPTION(webhooks);
#ifdef PA_SLEEPY
    PA_ADD_OPTION(integration);
#endif
}
void DiscordSettingsOption::load_json(const QJsonValue& json){
    BatchOption::load_json(json);

}





}
}

