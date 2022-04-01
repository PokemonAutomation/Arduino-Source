/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Options/ProcessPriorityOption.h"
#include "CommonFramework/Options/ProcessorLevelOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "Integrations/DiscordSettingsOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



class GlobalSettings : public BatchOption{
    GlobalSettings();
public:
    static GlobalSettings& instance();

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

public:
    BooleanCheckBoxOption SEND_ERROR_REPORTS;

    StringOption STATS_FILE;

    SimpleIntegerOption<uint32_t> WINDOW_WIDTH;
    SimpleIntegerOption<uint32_t> WINDOW_HEIGHT;

    SectionDividerOption m_discord_settings;
    Integration::DiscordSettingsOption DISCORD;

    SectionDividerOption m_advanced_options;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;
    BooleanCheckBoxOption NAUGHTY_MODE;

    ProcessorLevelOption PROCESSOR_LEVEL0;

//    ProcessPriorityOption PROCESS_PRIORITY0;
    ThreadPriorityOption REALTIME_THREAD_PRIORITY0;
    ThreadPriorityOption INFERENCE_PRIORITY0;
    ThreadPriorityOption COMPUTE_PRIORITY0;

    StringOption DEVELOPER_TOKEN;

    bool DEVELOPER_MODE;
};





class GlobalSettingsPanel;

class GlobalSettings_Descriptor : public PanelDescriptor{
public:
    GlobalSettings_Descriptor();
public:
    static PanelDescriptorWrapper<GlobalSettings_Descriptor, GlobalSettingsPanel> INSTANCE;
};


class GlobalSettingsPanel : public SettingsPanelInstance{
public:
    GlobalSettingsPanel(const GlobalSettings_Descriptor& descriptor);
private:
    GlobalSettings& settings;
};






}
#endif
