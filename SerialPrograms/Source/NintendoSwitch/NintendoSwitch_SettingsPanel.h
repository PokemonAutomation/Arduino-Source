/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SettingsPanel_H
#define PokemonAutomation_NintendoSwitch_SettingsPanel_H


#include "CommonFramework/Panels/OptionsPanel.h"
#include "NintendoSwitch_Settings.h"


namespace PokemonAutomation{
namespace NintendoSwitch{




class ConsoleSettings_Descriptor : public OptionsPanelDescriptor{
public:
    ConsoleSettings_Descriptor();
};


class ConsoleSettingsPanel : public OptionsPanelInstance{
public:
    using Descriptor = ConsoleSettings_Descriptor;
    ConsoleSettingsPanel();
private:
    ConsoleSettings& settings;
};



}
}
#endif
