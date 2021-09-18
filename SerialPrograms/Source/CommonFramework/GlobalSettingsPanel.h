/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Options/DiscordSettingsOption.h"
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

    StringOption STATS_FILE;

    SimpleIntegerOption<uint32_t> WINDOW_WIDTH;
    SimpleIntegerOption<uint32_t> WINDOW_HEIGHT;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;

    StringOption DEVELOPER_TOKEN;
};






}
#endif
