/*  Nintendo Switch Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Settings_H
#define PokemonAutomation_NintendoSwitch_Settings_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ConsoleSettings : public BatchOption{
    ConsoleSettings();
public:
    static ConsoleSettings& instance();

    TimeExpressionOption<uint16_t> CONNECT_CONTROLLER_DELAY;
    TimeExpressionOption<uint16_t> SETTINGS_TO_HOME_DELAY;
    BooleanCheckBoxOption START_GAME_REQUIRES_INTERNET;
    TimeExpressionOption<uint16_t> START_GAME_INTERNET_CHECK_DELAY;
    BooleanCheckBoxOption TOLERATE_SYSTEM_UPDATE_MENU_FAST;
    BooleanCheckBoxOption TOLERATE_SYSTEM_UPDATE_MENU_SLOW;
};





class ConsoleSettings_Descriptor : public PanelDescriptor{
public:
    ConsoleSettings_Descriptor();
};


class ConsoleSettingsPanel : public SettingsPanelInstance{
public:
    ConsoleSettingsPanel(const ConsoleSettings_Descriptor& descriptor);
private:
    ConsoleSettings& settings;
};



}
}
#endif
