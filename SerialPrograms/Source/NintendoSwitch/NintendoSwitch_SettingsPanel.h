/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SettingsPanel_H
#define PokemonAutomation_NintendoSwitch_SettingsPanel_H


#include "NintendoSwitch_Settings.h"
#include "CommonFramework/Panels/OptionsPanel.h"


namespace PokemonAutomation{
namespace NintendoSwitch{




class ConsoleSettings_Descriptor : public OptionsPanelDescriptor{
public:
    ConsoleSettings_Descriptor();
};


class ConsoleSettingsPanel : public OptionsPanelInstance{
public:
    ConsoleSettingsPanel();
private:
    ConsoleSettings& settings;
};



}
}
#endif
