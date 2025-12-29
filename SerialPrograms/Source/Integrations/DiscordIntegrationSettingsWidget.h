/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationSettingsWidget_H
#define PokemonAutomation_DiscordIntegrationSettingsWidget_H

#include "Common/Qt/Options/GroupWidget.h"
#include "DiscordIntegrationSettings.h"

namespace PokemonAutomation{
namespace Integration{



class DiscordIntegrationSettingsWidget : public GroupWidget{
public:
    using ParentOption = DiscordIntegrationSettingsOption;

public:
    DiscordIntegrationSettingsWidget(QWidget& parent, DiscordIntegrationSettingsOption& value);
};




}
}
#endif
