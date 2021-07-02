/*  Pokemon Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSettingsPanel_H
#define PokemonAutomation_PokemonSettingsPanel_H

#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokemonSettings_Descriptor : public PanelDescriptor{
public:
    PokemonSettings_Descriptor();
};


class PokemonSettings : public SettingsPanelInstance{
public:
    PokemonSettings(const PokemonSettings_Descriptor& descriptor);
};


}
}
}
#endif
