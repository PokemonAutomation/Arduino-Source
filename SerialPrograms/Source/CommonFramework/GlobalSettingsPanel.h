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
#include "CommonFramework/Options/Environment/ProcessPriorityOption.h"
#include "CommonFramework/Options/Environment/ProcessorLevelOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/VideoPipeline/CameraImplementations.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "Integrations/DiscordSettingsOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




class ResolutionOption : public GroupOption{
public:
    ResolutionOption(
        QString label, QString description,
        int default_width, int default_height
    );

    StaticTextOption DESCRIPTION;
    SimpleIntegerOption<uint32_t> WIDTH;
    SimpleIntegerOption<uint32_t> HEIGHT;
};






class GlobalSettings : public BatchOption{
    GlobalSettings();
public:
    static GlobalSettings& instance();

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

public:
    BooleanCheckBoxOption SEND_ERROR_REPORTS;

    StringOption STATS_FILE;
    ResolutionOption WINDOW_SIZE;
    ThemeSelectorOption THEME;

    SectionDividerOption m_discord_settings;
    Integration::DiscordSettingsOption DISCORD;

    SectionDividerOption m_advanced_options;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;
    BooleanCheckBoxOption NAUGHTY_MODE;

//    ProcessPriorityOption PROCESS_PRIORITY0;
    ThreadPriorityOption REALTIME_THREAD_PRIORITY0;
    ThreadPriorityOption INFERENCE_PRIORITY0;
    ThreadPriorityOption COMPUTE_PRIORITY0;

    VideoBackendOption VIDEO_BACKEND;
    BooleanCheckBoxOption ENABLE_FRAME_SCREENSHOTS;

    ProcessorLevelOption PROCESSOR_LEVEL0;

    StringOption DEVELOPER_TOKEN;

    // Developer mode shows some additional test programs used
    // for developing purposes.
    bool DEVELOPER_MODE = false;

    // The mode that does not run Qt GUI, but instead runs some tests for
    // debugging, unit testing and developing purposes.
    bool COMMAND_LINE_TEST_MODE = false;
    // Which tests to run if in the command line test mode.
    std::vector<std::string> COMMAND_LINE_TEST_LIST;
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
