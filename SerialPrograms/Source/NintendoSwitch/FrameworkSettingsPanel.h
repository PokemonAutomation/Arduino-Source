/*  Framework Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FrameworkSettingsPanel_H
#define PokemonAutomation_FrameworkSettingsPanel_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FrameworkSettings_Descriptor : public PanelDescriptor{
public:
    FrameworkSettings_Descriptor();
};



class FrameworkSettings : public SettingsPanelInstance{
public:
    FrameworkSettings(const FrameworkSettings_Descriptor& descriptor);

    TimeExpressionOption<uint16_t> CONNECT_CONTROLLER_DELAY;
    TimeExpressionOption<uint16_t> SETTINGS_TO_HOME_DELAY;
    BooleanCheckBoxOption START_GAME_REQUIRES_INTERNET;
    TimeExpressionOption<uint16_t> START_GAME_INTERNET_CHECK_DELAY;
    BooleanCheckBoxOption TOLERATE_SYSTEM_UPDATE_MENU_FAST;
    BooleanCheckBoxOption TOLERATE_SYSTEM_UPDATE_MENU_SLOW;
};



}
}
#endif
