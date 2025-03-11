/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DiscordSettings_H
#define PokemonAutomation_DiscordSettings_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "DiscordWebhookSettings.h"
#include "DiscordIntegrationSettings.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordMessageSettingsOption : public BatchOption{
public:
    DiscordMessageSettingsOption();
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

    StringOption instance_name;
    StringOption user_id;
    StringOption message;
};






class DiscordSettingsOption : public BatchOption{
public:
    DiscordSettingsOption();

    DiscordMessageSettingsOption message;
    DiscordWebhookSettingsOption webhooks;
    DiscordIntegrationSettingsOption integration;
};




}
}
#endif
