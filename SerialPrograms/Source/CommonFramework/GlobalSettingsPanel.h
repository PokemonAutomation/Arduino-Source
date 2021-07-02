/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{

class GlobalSettings;

class GlobalSettings_Descriptor : public PanelDescriptor{
public:
    GlobalSettings_Descriptor();
public:
    static PanelDescriptorWrapper<GlobalSettings_Descriptor, GlobalSettings> INSTANCE;
};


class GlobalSettings : public SettingsPanelInstance{
public:
    GlobalSettings(const GlobalSettings_Descriptor& descriptor);
};






}
#endif
