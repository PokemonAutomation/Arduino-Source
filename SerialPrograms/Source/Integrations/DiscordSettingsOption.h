/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordSettings_H
#define PokemonAutomation_DiscordSettings_H

#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "DiscordWebhookSettings.h"
#include "DiscordIntegrationSettings.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordMessageSettingsOption : public BatchOption{
public:
    DiscordMessageSettingsOption();
    virtual ConfigWidget* make_ui(QWidget& parent) override;

    StringOption instance_name;
    StringOption user_id;
    StringOption message;
};
class DiscordMessageSettingsOptionUI : public BatchWidget{
public:
    DiscordMessageSettingsOptionUI(QWidget& parent, DiscordMessageSettingsOption& value);
};
inline ConfigWidget* DiscordMessageSettingsOption::make_ui(QWidget& parent){
    return new DiscordMessageSettingsOptionUI(parent, *this);
}








class DiscordSettingsOption : public BatchOption{
public:
    DiscordSettingsOption();
//    virtual void load_json(const JsonValueRef& json) override;

    DiscordMessageSettingsOption message;
    DiscordWebhookSettingsOption webhooks;
    DiscordIntegrationSettingsOption integration;
};




}
}
#endif
