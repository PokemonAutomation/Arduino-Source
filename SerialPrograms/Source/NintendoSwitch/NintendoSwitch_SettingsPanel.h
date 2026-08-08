/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SettingsPanel_H
#define PokemonAutomation_NintendoSwitch_SettingsPanel_H


#include "NintendoSwitch_Settings.h"
#include "CommonFramework/Panels/SettingsPanel.h"


namespace PokemonAutomation{
namespace NintendoSwitch{




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
