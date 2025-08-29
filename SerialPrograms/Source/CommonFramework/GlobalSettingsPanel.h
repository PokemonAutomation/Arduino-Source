/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GlobalSettingsPanel_H
#define PokemonAutomation_GlobalSettingsPanel_H

#include <vector>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "CommonFramework/Panels/PanelTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

class CheckForUpdatesOption;
class ThemeSelectorOption;
class ResolutionOption;
class StreamHistoryOption;
class SleepSuppressOptions;
namespace Integration{
    class DiscordSettingsOption;
}
class PerformanceOptions;
class AudioPipelineOptions;
class VideoPipelineOptions;
class ErrorReportOption;




class FolderInputOption : public StringOption{
public:
    using StringOption::StringOption;

    virtual void sanitize(std::string& str) override{
        if (!str.empty() && str.back() != '/'){
            str += '/';
        }
    }
};


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
    virtual void on_config_value_changed(void* object) override;

public:
    Pimpl<CheckForUpdatesOption> CHECK_FOR_UPDATES;

    StringOption STATS_FILE;
    FolderInputOption TEMP_FOLDER;

    Pimpl<ThemeSelectorOption> THEME;
    Pimpl<ResolutionOption> WINDOW_SIZE;
    Pimpl<ResolutionOption> LOG_WINDOW_SIZE;
    BooleanCheckBoxOption LOG_WINDOW_STARTUP;

    Pimpl<StreamHistoryOption> STREAM_HISTORY;
    Pimpl<SleepSuppressOptions> SLEEP_SUPPRESS;

    SectionDividerOption m_discord_settings;
    BooleanCheckBoxOption RICH_PRESENCE;
    BooleanCheckBoxOption ALL_STATS;
    Pimpl<Integration::DiscordSettingsOption> DISCORD;

    SectionDividerOption m_advanced_options;

    BooleanCheckBoxOption LOG_EVERYTHING;
    BooleanCheckBoxOption SAVE_DEBUG_IMAGES;
//    BooleanCheckBoxOption NAUGHTY_MODE_OPTION;
    BooleanCheckBoxOption HIDE_NOTIF_DISCORD_LINK;

    Pimpl<PerformanceOptions> PERFORMANCE;
    Pimpl<AudioPipelineOptions> AUDIO_PIPELINE;
    Pimpl<VideoPipelineOptions> VIDEO_PIPELINE;

    BooleanCheckBoxOption ENABLE_LIFETIME_SANITIZER0;

    Pimpl<ErrorReportOption> ERROR_REPORTS;

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
