/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <set>
#include <QStandardPaths>
#include <QCryptographicHash>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/CheckForUpdatesOption.h"
#include "CommonFramework/Options/ResolutionOption.h"
#include "CommonFramework/Options/Environment/SleepSuppressOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "Integrations/DiscordSettingsOption.h"
//#include "CommonFramework/Environment/Environment.h"
#include "GlobalSettingsPanel.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{


const std::set<std::string> TOKENS{
//    "f6538243092d8a3b9959bca988f054e1670f57c7246df2cbba25c4df3fe7a4e7",
    "2d04af67f6520e3550842d7eeb292868c6d0d4809b607f5a454712023d8815e1",
    "475d0a0a305a02cbf8b602bd47c3b275dccd5ac19fbe480729804a8e4e360b71",
    "6643d9fe87b3e54dc75dfac8ac22f0cc8bd17f6a8a786debf5fc4c517ee65469",
    "8e48e38e49bffc8462ada9d2d9d850d5b3b5c9529d20978c09bc548bc9a614a4",
    "7694adee4419d62c6a923c4efc9e7b41def7b96bb84ea882701b0bf2e8c13bee",
    "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08", //  jw's token.
    "e8d168bc482e96553ea9f9ecaea5a817474dbccc2a6a228a6bde67f2b2aa2889", //  James' token.
    "7555b7c63481cad42306718c67e7f9def5bfd1da8f6cd299ccd3d7dc95f307ae", //  Kuro's token.
    "3d475b46d121fc24559d100de2426feaa53cd6578aac2817c4857a610ccde2dd", //  kichi's token.
};





PreloadSettings::PreloadSettings(){}
PreloadSettings& PreloadSettings::instance(){
    static PreloadSettings settings;
    return settings;
}
DebugSettings& PreloadSettings::debug(){
    return PreloadSettings::instance().DEBUG;
}

void PreloadSettings::load(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    //  Naughty mode.
    obj->read_boolean(NAUGHTY_MODE, "NAUGHTY_MODE");

    //  Developer mode stuff.
    const std::string* dev_token = obj->get_string("DEVELOPER_TOKEN");
    if (dev_token){
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
#if QT_VERSION < 0x060700
        hash.addData(dev_token->c_str(), (int)dev_token->size());
#else
        QByteArrayView dataView(dev_token->data(), dev_token->size());
        hash.addData(dataView);
#endif
        DEVELOPER_MODE = TOKENS.find(hash.result().toHex().toStdString()) != TOKENS.end();
    }

    const JsonObject* debug_obj = obj->get_object("DEBUG");
    if (debug_obj){
        debug_obj->read_boolean(DEBUG.COLOR_CHECK, "COLOR_CHECK");
        debug_obj->read_boolean(DEBUG.IMAGE_TEMPLATE_MATCHING, "IMAGE_TEMPLATE_MATCHING");
        debug_obj->read_boolean(DEBUG.IMAGE_DICTIONARY_MATCHING, "IMAGE_DICTIONARY_MATCHING");
    }
}




GlobalSettings& GlobalSettings::instance(){
    static GlobalSettings settings;
    return settings;
}
GlobalSettings::~GlobalSettings(){
    ENABLE_LIFETIME_SANITIZER0.remove_listener(*this);
}
GlobalSettings::GlobalSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , CHECK_FOR_UPDATES(CONSTRUCT_TOKEN)
    , STATS_FILE(
        false,
        "<b>Stats File:</b><br>Use the stats file here. Multiple instances of the program can use the same file.",
        LockMode::LOCK_WHILE_RUNNING,
#if defined(__APPLE__)
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/UserSettings/PA-Stats.txt",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/UserSettings/PA-Stats.txt"
#else
        "UserSettings/PA-Stats.txt",
        "UserSettings/PA-Stats.txt"
#endif
    )
    , TEMP_FOLDER(
        false,
        "<b>Temp Folder:</b><br>Place temporary files in this directory.",
        LockMode::LOCK_WHILE_RUNNING,
#if defined(__APPLE__)
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/TempFiles/",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/TempFiles/"
#else
        "TempFiles/",
        "TempFiles/"
#endif
    )
    , THEME(CONSTRUCT_TOKEN)
    , WINDOW_SIZE(
        CONSTRUCT_TOKEN,
        "Window Size/Position:",
        "Set the size/position of the window. Takes effect immediately.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.",
        1280, 1000,
        0, 0
    )
    , LOG_WINDOW_SIZE(
        CONSTRUCT_TOKEN,
        "Output Window Size/Position:",
        "Set the size/position of the output window. Takes effect immediately.<br>",
        600, 1200,
        0, 0
    )
    , LOG_WINDOW_STARTUP(
        "<b>Open Output Window at startup:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , STREAM_HISTORY(CONSTRUCT_TOKEN)
    , SLEEP_SUPPRESS(CONSTRUCT_TOKEN)
    , m_discord_settings(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. (" +
        make_text_url(
            ONLINE_DOC_URL_BASE + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md",
            "online documentation"
        ) + ")</font>"
    )
    , RICH_PRESENCE(
        "<b>Enable Rich Presence:</b><br>"
        "Display program activity and status under your Discord user.<br>"
        "Restart the program for the change to take effect.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , ALL_STATS(
        "<b>All Stats:</b><br>Include all-time stats for notifications.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , DISCORD(CONSTRUCT_TOKEN)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , LOG_EVERYTHING(
        "<b>Log Everything:</b><br>Log everything to the output window and output log. Will be very spammy.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SAVE_DEBUG_IMAGES(
        "<b>Save Debug Images:</b><br>"
        "If the program fails to read something when it should succeed, save the image for debugging purposes.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
//    , NAUGHTY_MODE_OPTION("<b>Naughty Mode:</b>", false)
    , HIDE_NOTIF_DISCORD_LINK(
        "<b>Hide Discord Link in Notifications:</b><br>"
        "Many Discord servers have rules forbidding links to other Discord servers. "
        "Checking this box will hide the support link that appears in the footer of every Discord notification.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , PERFORMANCE(CONSTRUCT_TOKEN)
    , AUDIO_PIPELINE(CONSTRUCT_TOKEN)
    , VIDEO_PIPELINE(CONSTRUCT_TOKEN)
    , ENABLE_LIFETIME_SANITIZER0(
        "<b>Enable Lifetime Sanitizer: (for debugging)</b><br>"
        "Check for C++ object lifetime violations. Terminate program with stack dump if violations are found. "
        "If enabling, you must restart the program for it to take effect.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
//        IS_BETA_VERSION
    )
    , ERROR_REPORTS(CONSTRUCT_TOKEN)
    , DEVELOPER_TOKEN(
        true,
        "<b>Developer Token:</b><br>Restart application to take full effect after changing this.",
        LockMode::LOCK_WHILE_RUNNING,
        "", ""
    )
{
    PA_ADD_OPTION(CHECK_FOR_UPDATES);
    PA_ADD_OPTION(STATS_FILE);
    PA_ADD_OPTION(TEMP_FOLDER);
    PA_ADD_OPTION(THEME);
    PA_ADD_OPTION(WINDOW_SIZE);
    PA_ADD_OPTION(LOG_WINDOW_SIZE);
    PA_ADD_OPTION(LOG_WINDOW_STARTUP);
#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(STREAM_HISTORY);
    }
#else
    STREAM_HISTORY->set_enabled(false);
#endif
#ifdef PA_ENABLE_SLEEP_SUPPRESS
    PA_ADD_OPTION(SLEEP_SUPPRESS);
#endif

    PA_ADD_STATIC(m_discord_settings);
#ifdef PA_SOCIAL_SDK
    PA_ADD_OPTION(RICH_PRESENCE);
#endif
    PA_ADD_OPTION(ALL_STATS);
    PA_ADD_OPTION(DISCORD);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(LOG_EVERYTHING);
    PA_ADD_OPTION(SAVE_DEBUG_IMAGES);
//    PA_ADD_OPTION(NAUGHTY_MODE);
    PA_ADD_OPTION(HIDE_NOTIF_DISCORD_LINK);

    PA_ADD_OPTION(PERFORMANCE);

    PA_ADD_OPTION(AUDIO_PIPELINE);
    PA_ADD_OPTION(VIDEO_PIPELINE);

    PA_ADD_OPTION(ENABLE_LIFETIME_SANITIZER0);

#ifdef PA_OFFICIAL
    PA_ADD_OPTION(ERROR_REPORTS);
#endif

    PA_ADD_OPTION(DEVELOPER_TOKEN);

    GlobalSettings::on_config_value_changed(this);
    ENABLE_LIFETIME_SANITIZER0.add_listener(*this);
}

void GlobalSettings::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    PreloadSettings::instance().load(json);

    BatchOption::load_json(json);

    //  Remake this to update the color.
    m_discord_settings.set_text(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. (" +
        make_text_url(
            ONLINE_DOC_URL_BASE + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md",
            "online documentation"
        ) + ")</font>"
    );

    COMMAND_LINE_TEST_LIST.clear();
    COMMAND_LINE_IGNORE_LIST.clear();
    const JsonObject* command_line_tests_setting = obj->get_object("COMMAND_LINE_TESTS");
    if (command_line_tests_setting){
        command_line_tests_setting->read_boolean(COMMAND_LINE_TEST_MODE, "RUN");

        if (!command_line_tests_setting->read_string(COMMAND_LINE_TEST_FOLDER, "FOLDER")){
            COMMAND_LINE_TEST_FOLDER = "CommandLineTests";
        }

        const JsonArray* test_list = command_line_tests_setting->get_array("TEST_LIST");
        if (test_list){
            for (const auto& value: *test_list){
                if (!value.is_string()){
                    continue;
                }
                const std::string* test_name = value.to_string();
                if (test_name != nullptr && !test_name->empty()){
                    COMMAND_LINE_TEST_LIST.emplace_back(*test_name);
                }
            }
        }
        const JsonArray* ignore_list = command_line_tests_setting->get_array("IGNORE_LIST");
        if (ignore_list){
            for (const auto& value: *ignore_list){
                if (!value.is_string()){
                    continue;
                }
                const std::string* test_name = value.to_string();
                if (test_name != nullptr && !test_name->empty()){
                    COMMAND_LINE_IGNORE_LIST.emplace_back(*test_name);
                }
            }
        }

        if (COMMAND_LINE_TEST_MODE){
            std::cout << "Enter command line test mode:" << std::endl;
            if (COMMAND_LINE_TEST_LIST.size() > 0){
                std::cout << "Run following tests: " << std::endl;
                for(const auto& name : COMMAND_LINE_TEST_LIST){
                    std::cout << "- " << name << std::endl;
                }
            }
            if (COMMAND_LINE_IGNORE_LIST.size() > 0){
                std::cout << "Ignore following " << COMMAND_LINE_IGNORE_LIST.size() << " paths: " << std::endl;
                const size_t MAX_LINES = 5;
                for(size_t i = 0; i < COMMAND_LINE_IGNORE_LIST.size() && i < MAX_LINES; i++){
                    std::cout << "- " << COMMAND_LINE_IGNORE_LIST[i] << std::endl;
                }
                if (COMMAND_LINE_IGNORE_LIST.size() > MAX_LINES){
                    std::cout << "..." << std::endl;
                }
            }
        }
    }
}


JsonValue GlobalSettings::to_json() const{
    JsonObject obj = std::move(*BatchOption::to_json().to_object());
    obj["NAUGHTY_MODE"] = PreloadSettings::instance().NAUGHTY_MODE;

    JsonObject command_line_test_obj;
    command_line_test_obj["RUN"] = COMMAND_LINE_TEST_MODE;
    command_line_test_obj["FOLDER"] = COMMAND_LINE_TEST_FOLDER;

    {
        JsonArray test_list;
        for(const auto& name : COMMAND_LINE_TEST_LIST){
            test_list.push_back(name);
        }
        command_line_test_obj["TEST_LIST"] = std::move(test_list);
    }

    {
        JsonArray ignore_list;
        for(const auto& name : COMMAND_LINE_IGNORE_LIST){
            ignore_list.push_back(name);
        }
        command_line_test_obj["IGNORE_LIST"] = std::move(ignore_list);
    }

    obj["COMMAND_LINE_TESTS"] = std::move(command_line_test_obj);

    JsonObject debug_obj;
    const auto& debug_settings = PreloadSettings::instance().DEBUG;
    debug_obj["COLOR_CHECK"] = debug_settings.COLOR_CHECK;
    debug_obj["IMAGE_TEMPLATE_MATCHING"] = debug_settings.IMAGE_TEMPLATE_MATCHING;
    debug_obj["IMAGE_DICTIONARY_MATCHING"] = debug_settings.IMAGE_DICTIONARY_MATCHING;
    obj["DEBUG"] = std::move(debug_obj);

    return obj;
}

void GlobalSettings::on_config_value_changed(void* object){
    bool enabled = ENABLE_LIFETIME_SANITIZER0;
    if (enabled){
        global_logger_tagged().log("LifeTime Sanitizer: Enabled", COLOR_BLUE);
    }else{
        global_logger_tagged().log("LifeTime Sanitizer: Disabled", COLOR_BLUE);
        LifetimeSanitizer::disable();
    }
}






PanelDescriptorWrapper<GlobalSettings_Descriptor, GlobalSettingsPanel> GlobalSettings_Descriptor::INSTANCE;

GlobalSettings_Descriptor::GlobalSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "",
        "Global Settings", "Global Settings",
        "",
        "Global Settings"
    )
{}


GlobalSettingsPanel::GlobalSettingsPanel(const GlobalSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , settings(GlobalSettings::instance())
{
    PA_ADD_OPTION(settings);
}


}
