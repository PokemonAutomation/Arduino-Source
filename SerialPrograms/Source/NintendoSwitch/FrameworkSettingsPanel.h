/*  Framework Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FrameworkSettingsPanel_H
#define PokemonAutomation_FrameworkSettingsPanel_H

#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FrameworkSettings_Descriptor : public PanelDescriptor{
public:
    FrameworkSettings_Descriptor();
};



class FrameworkSettings : public SettingsPanelInstance{
public:
    FrameworkSettings(const FrameworkSettings_Descriptor& descriptor);
};



}
}
#endif
