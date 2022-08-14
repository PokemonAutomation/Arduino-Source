/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "Common/Qt/Options/FloatingPointOption.h"
#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "CommonFramework/Options/Environment/ProcessPriorityOption.h"
#include "CommonFramework/Options/Environment/ProcessorLevelOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "Integrations/DiscordSettingsOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




class ResolutionOption : public GroupOption{
public:
    ResolutionOption(
        std::string label, std::string description,
        int default_width, int default_height
    );

    StaticTextOption DESCRIPTION;
    SimpleIntegerOption<uint32_t> WIDTH;
    SimpleIntegerOption<uint32_t> HEIGHT;
};




class PreloadSettings{
    PreloadSettings();
public:
    static PreloadSettings& instance();

    void load(const JsonValue& json);

    bool NAUGHTY_MODE = false;
    bool DEVELOPER_MODE = false;
};



class GlobalSettings : public BatchOption{
    GlobalSettings();
public:
    static GlobalSettings& instance();

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    BooleanCheckBoxOption SEND_ERROR_REPORTS;

    StringOption STATS_FILE;
    BooleanCheckBoxOption ALL_STATS;

    ResolutionOption WINDOW_SIZE;
    ThemeSelectorOption THEME;

    SectionDividerOption m_discord_settings;
    Integration::DiscordSettingsOption DISCORD;

    SectionDividerOption m_advanced_options;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;
//    BooleanCheckBoxOption NAUGHTY_MODE_OPTION;

//    ProcessPriorityOption PROCESS_PRIORITY0;
    ThreadPriorityOption REALTIME_THREAD_PRIORITY0;
    ThreadPriorityOption INFERENCE_PRIORITY0;
    ThreadPriorityOption COMPUTE_PRIORITY0;

    FloatingPointOption AUDIO_FILE_VOLUME_SCALE;
    FloatingPointOption AUDIO_DEVICE_VOLUME_SCALE;
    BooleanCheckBoxOption SHOW_ALL_AUDIO_DEVICES;
    BooleanCheckBoxOption SHOW_RECORD_FREQUENCIES;
    VideoBackendOption VIDEO_BACKEND;
    BooleanCheckBoxOption ENABLE_FRAME_SCREENSHOTS;

    ProcessorLevelOption PROCESSOR_LEVEL0;

    StringOption DEVELOPER_TOKEN;

    // The mode that does not run Qt GUI, but instead runs some tests for
    // debugging, unit testing and developing purposes.
    bool COMMAND_LINE_TEST_MODE = false;
    // The path to the command line test folder.
    std::string COMMAND_LINE_TEST_FOLDER;
    // Which tests to run if in the command line test mode.
    std::vector<std::string> COMMAND_LINE_TEST_LIST;
    // Which tests to ignore running under the command line test mode.
    // If a test path appears in both COMMAND_LINE_TEST_LIST and COMMAND_LINE_IGNORE_LIST, it's still ignored.
    std::vector<std::string> COMMAND_LINE_IGNORE_LIST;
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
