/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Options/ResolutionOption.h"
#include "CommonFramework/Options/Environment/ProcessPriorityOption.h"
#include "CommonFramework/Options/Environment/ProcessorLevelOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Options/Environment/SleepSuppressOption.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "Integrations/DiscordSettingsOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




struct DebugSettings{
    bool COLOR_CHECK = false;
    bool IMAGE_TEMPLATE_MATCHING = false;
    bool IMAGE_DICTIONARY_MATCHING = false;
};



class PreloadSettings{
    PreloadSettings();
public:
    static PreloadSettings& instance();
    static DebugSettings& debug();

    void load(const JsonValue& json);

    bool NAUGHTY_MODE = false;
    bool DEVELOPER_MODE = false;

    DebugSettings DEBUG;
};



class GlobalSettings : public BatchOption, private ConfigOption::Listener{
    ~GlobalSettings();
    GlobalSettings();
public:
    static GlobalSettings& instance();

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

private:
    virtual void value_changed(void* object) override;

public:
    BooleanCheckBoxOption CHECK_FOR_UPDATES;

    StringOption STATS_FILE;
    BooleanCheckBoxOption ALL_STATS;

    ThemeSelectorOption THEME;
    ResolutionOption WINDOW_SIZE;

    StreamHistoryOption STREAM_HISTORY;
    SleepSuppressOptions SLEEP_SUPPRESS;

    SectionDividerOption m_discord_settings;
    Integration::DiscordSettingsOption DISCORD;

    SectionDividerOption m_advanced_options;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;
//    BooleanCheckBoxOption NAUGHTY_MODE_OPTION;

    BooleanCheckBoxOption HIDE_NOTIF_DISCORD_LINK;

//    ProcessPriorityOption PROCESS_PRIORITY0;
    ThreadPriorityOption REALTIME_THREAD_PRIORITY0;
    ThreadPriorityOption INFERENCE_PRIORITY0;
    ThreadPriorityOption COMPUTE_PRIORITY0;

    FloatingPointOption AUDIO_FILE_VOLUME_SCALE;
    FloatingPointOption AUDIO_DEVICE_VOLUME_SCALE;
    BooleanCheckBoxOption SHOW_ALL_AUDIO_DEVICES;
    BooleanCheckBoxOption SHOW_RECORD_FREQUENCIES;

    VideoBackendOption VIDEO_BACKEND;
#if QT_VERSION_MAJOR == 5
    BooleanCheckBoxOption ENABLE_FRAME_SCREENSHOTS;
#endif

    SimpleIntegerOption<uint8_t> AUTO_RESET_AUDIO_SECONDS;
    SimpleIntegerOption<uint8_t> AUTO_RESET_VIDEO_SECONDS;

    BooleanCheckBoxOption ENABLE_LIFETIME_SANITIZER;

//    BooleanCheckBoxOption SEND_ERROR_REPORTS0;
    ErrorReportOption ERROR_REPORTS;

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
