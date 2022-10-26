/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <set>
#include <QCryptographicHash>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "GlobalSettingsPanel.h"

namespace PokemonAutomation{


const std::set<std::string> TOKENS{
    "f6538243092d8a3b9959bca988f054e1670f57c7246df2cbba25c4df3fe7a4e7",
    "2d04af67f6520e3550842d7eeb292868c6d0d4809b607f5a454712023d8815e1",
    "475d0a0a305a02cbf8b602bd47c3b275dccd5ac19fbe480729804a8e4e360b71",
    "6643d9fe87b3e54dc75dfac8ac22f0cc8bd17f6a8a786debf5fc4c517ee65469",
    "8e48e38e49bffc8462ada9d2d9d850d5b3b5c9529d20978c09bc548bc9a614a4",
    "7694adee4419d62c6a923c4efc9e7b41def7b96bb84ea882701b0bf2e8c13bee",
};



ResolutionOption::ResolutionOption(
    std::string label, std::string description,
    int default_width, int default_height
)
    : GroupOption(std::move(label), LockWhileRunning::LOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , WIDTH("<b>Width:</b>", scale_dpi_width(default_width))
    , HEIGHT("<b>Height:</b>", scale_dpi_height(default_height))
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
}




PreloadSettings::PreloadSettings(){}
PreloadSettings& PreloadSettings::instance(){
    static PreloadSettings settings;
    return settings;
}
DebugSettings& PreloadSettings::debug(){
    return PreloadSettings::instance().DEBUG;
}

void PreloadSettings::load(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    //  Naughty mode.
    obj->read_boolean(NAUGHTY_MODE, "NAUGHTY_MODE");

    //  Developer mode stuff.
    const std::string* dev_token = obj->get_string("DEVELOPER_TOKEN");
    if (dev_token){
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
        hash.addData(dev_token->c_str(), (int)dev_token->size());
        DEVELOPER_MODE = TOKENS.find(hash.result().toHex().toStdString()) != TOKENS.end();
    }

    const JsonObject* debug_obj = obj->get_object("DEBUG");
    if (debug_obj){
        debug_obj->read_boolean(DEBUG.COLOR_CHECK, "COLOR_CHECK");
        debug_obj->read_boolean(DEBUG.IMAGE_TEMPLATE_MATCHING, "IMAGE_TEMPLATE_MATCHING");
    }
}




GlobalSettings& GlobalSettings::instance(){
    static GlobalSettings settings;
    return settings;
}
GlobalSettings::GlobalSettings()
    : BatchOption(LockWhileRunning::LOCK_WHILE_RUNNING)
    , SEND_ERROR_REPORTS(
        "<b>Send Error Reports:</b><br>"
        "Send error reports to the " + PROGRAM_NAME + " server to help them resolve issues and improve the program.",
        false
    )
    , STATS_FILE(
        false,
        "<b>Stats File:</b><br>Use the stats file here. Multiple instances of the program can use the same file.",
        "PA-Stats.txt",
        "PA-Stats.txt"
    )
    , ALL_STATS(
        "<b>All Stats:</b><br>Include all-time stats for notifications.",
        true
    )
    , WINDOW_SIZE(
        "Window Size:",
        "Set the size of the window. Restart application to take effect.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.",
        1280, 720
    )
    , m_discord_settings(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. (" +
        make_text_url(
            ONLINE_DOC_URL + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md",
            "online documentation"
        ) + ")</font>"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , LOG_EVERYTHING(
        "<b>Log Everything:</b><br>Log everything to the output window and output log. Will be very spammy.",
        false
    )
    , SAVE_DEBUG_IMAGES(
        "<b>Save Debug Images:</b><br>"
        "If the program fails to read something when it should succeed, save the image for debugging purposes.",
        true
    )
//    , NAUGHTY_MODE_OPTION("<b>Naughty Mode:</b>", false)
    , REALTIME_THREAD_PRIORITY0(
        "<b>Realtime Thread Priority:</b><br>"
        "Thread priority of real-time threads. (UI thread, audio threads)<br>"
        "Restart the program for this to fully take effect.",
        DEFAULT_PRIORITY_REALTIME
    )
    , INFERENCE_PRIORITY0(
        "<b>Inference Priority:</b><br>"
        "Thread priority of inference threads. (image/sound recognition)",
        DEFAULT_PRIORITY_INFERENCE
    )
    , COMPUTE_PRIORITY0(
        "<b>Compute Priority:</b><br>"
        "Thread priority of computation threads.",
        DEFAULT_PRIORITY_COMPUTE
    )
    , AUDIO_FILE_VOLUME_SCALE(
        "<b>Audio File Input Volume Scale:</b><br>"
        "Multiply audio file playback by this factor. (This is linear scale. So each factor of 10 is 20dB.)",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        0.31622776601683793320, //  -10dB
        -10000, 10000
    )
    , AUDIO_DEVICE_VOLUME_SCALE(
        "<b>Audio Device Input Volume Scale:</b><br>"
        "Multiply audio device input by this factor. (This is linear scale. So each factor of 10 is 20dB.)",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        1.0, -10000, 10000
    )
    , SHOW_ALL_AUDIO_DEVICES(
        "<b>Show all Audio Devices:</b><br>"
        "Show all audio devices - including duplicates.",
        false
    )
    , SHOW_RECORD_FREQUENCIES(
        "<b>Show Record Frequencies:</b><br>"
        "Show option to record audio frequencies.",
        false
    )
    , ENABLE_FRAME_SCREENSHOTS(
        "<b>Enable Frame Screenshots:</b><br>"
        "Attempt to use QVideoProbe and QVideoFrame for screenshots.",
        true
    )
    , DEVELOPER_TOKEN(
        true,
        "<b>Developer Token:</b><br>Restart application to take full effect after changing this.",
        "", ""
    )
{
    PA_ADD_OPTION(SEND_ERROR_REPORTS);
    PA_ADD_OPTION(STATS_FILE);
    PA_ADD_OPTION(ALL_STATS);
    PA_ADD_OPTION(WINDOW_SIZE);
    PA_ADD_OPTION(THEME);

    PA_ADD_STATIC(m_discord_settings);
    PA_ADD_OPTION(DISCORD);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(LOG_EVERYTHING);
    PA_ADD_OPTION(SAVE_DEBUG_IMAGES);
//    PA_ADD_OPTION(NAUGHTY_MODE);

    PA_ADD_OPTION(REALTIME_THREAD_PRIORITY0);
    PA_ADD_OPTION(INFERENCE_PRIORITY0);
    PA_ADD_OPTION(COMPUTE_PRIORITY0);

    PA_ADD_OPTION(AUDIO_FILE_VOLUME_SCALE);
    PA_ADD_OPTION(AUDIO_DEVICE_VOLUME_SCALE);
    PA_ADD_OPTION(SHOW_ALL_AUDIO_DEVICES);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SHOW_RECORD_FREQUENCIES);
    }
    PA_ADD_OPTION(VIDEO_BACKEND);
#if QT_VERSION_MAJOR == 5
    PA_ADD_OPTION(ENABLE_FRAME_SCREENSHOTS);
#endif

    PA_ADD_OPTION(PROCESSOR_LEVEL0);

    PA_ADD_OPTION(DEVELOPER_TOKEN);
}

void GlobalSettings::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    PreloadSettings::instance().load(json);

    BatchOption::load_json(json);

    //  Remake this to update the color.
    m_discord_settings.set_text(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. (" +
        make_text_url(
            ONLINE_DOC_URL + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md",
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
                const std::string* test_name = value.get_string();
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
                const std::string* test_name = value.get_string();
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
                std::cout << "Ignore following paths: " << std::endl;
                for(const auto& name : COMMAND_LINE_IGNORE_LIST){
                    std::cout << "- " << name << std::endl;
                }
            }
        }
    }
}


JsonValue GlobalSettings::to_json() const{
    JsonObject obj = std::move(*BatchOption::to_json().get_object());
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
    obj["DEBUG"] = std::move(debug_obj);

    return obj;
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
