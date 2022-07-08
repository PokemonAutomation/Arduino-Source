/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <set>
#include <QCryptographicHash>
#include "Common/Qt/QtJsonTools.h"
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
    QString label, QString description,
    int default_width, int default_height
)
    : GroupOption(std::move(label))
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
void PreloadSettings::load(const QJsonValue& json){
    const QJsonObject obj = json.toObject();

    //  Naughty mode.
    json_get_bool(NAUGHTY_MODE, obj, "NAUGHTY_MODE");

    //  Developer mode stuff.
    QString dev_token;
    json_get_string(dev_token, obj, "DEVELOPER_TOKEN");
    {
        std::string token = dev_token.toStdString();
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
        hash.addData(token.c_str(), (int)token.size());
        DEVELOPER_MODE = TOKENS.find(hash.result().toHex().toStdString()) != TOKENS.end();
    }
}




GlobalSettings& GlobalSettings::instance(){
    static GlobalSettings settings;
    return settings;
}
GlobalSettings::GlobalSettings()
    : SEND_ERROR_REPORTS(
        "<b>Send Error Reports:</b><br>"
        "Send error reports to the " + STRING_POKEMON + " Automation server to help them resolve issues and improve the program.",
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
        2
    )
    , INFERENCE_PRIORITY0(
        "<b>Inference Priority:</b><br>"
        "Thread priority of inference threads. (image/sound recognition)",
        1
    )
    , COMPUTE_PRIORITY0(
        "<b>Compute Priority:</b><br>"
        "Thread priority of computation threads.",
        -1
    )
    , SHOW_ALL_AUDIO_DEVICES(
        "<b>Show all Audio Devices:</b><br>"
        "Show all audio devices - including duplicates.",
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

    PA_ADD_OPTION(SHOW_ALL_AUDIO_DEVICES);
    PA_ADD_OPTION(VIDEO_BACKEND);
#if QT_VERSION_MAJOR == 5
    PA_ADD_OPTION(ENABLE_FRAME_SCREENSHOTS);
#endif

    PA_ADD_OPTION(PROCESSOR_LEVEL0);

    PA_ADD_OPTION(DEVELOPER_TOKEN);
}

void GlobalSettings::load_json(const QJsonValue& json){
    const QJsonObject obj = json.toObject();

    PreloadSettings::instance().load(json);

    BatchOption::load_json(json);

    //  Remake this to update the color.
    m_discord_settings = SectionDividerOption(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. (" +
        make_text_url(
            ONLINE_DOC_URL + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md",
            "online documentation"
        ) + ")</font>"
    );

    COMMAND_LINE_TEST_LIST.clear();
    COMMAND_LINE_IGNORE_LIST.clear();
    const QJsonObject command_line_tests_setting = json_get_object_nothrow(obj, "COMMAND_LINE_TESTS");
    if (!command_line_tests_setting.isEmpty()){
        json_get_bool(COMMAND_LINE_TEST_MODE, command_line_tests_setting, "RUN");

        QString folder;
        if (json_get_string(folder, command_line_tests_setting, "FOLDER")){
            COMMAND_LINE_TEST_FOLDER = folder.toStdString();
        } else{
            COMMAND_LINE_TEST_FOLDER = "CommandLineTests";
        }

        const QJsonArray test_list = json_get_array_nothrow(command_line_tests_setting, "TEST_LIST");
        for(const auto& value: test_list){
            const std::string test_name = value.toString().toStdString();
            if (test_name.size() > 0){
                COMMAND_LINE_TEST_LIST.emplace_back(std::move(test_name));
            }
        }
        const QJsonArray ignore_list = json_get_array_nothrow(command_line_tests_setting, "IGNORE_LIST");
        for(const auto& value: ignore_list){
            const std::string test_name = value.toString().toStdString();
            if (test_name.size() > 0){
                COMMAND_LINE_IGNORE_LIST.emplace_back(std::move(test_name));
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


QJsonValue GlobalSettings::to_json() const{
    QJsonObject obj = BatchOption::to_json().toObject();
    obj.insert("NAUGHTY_MODE", PreloadSettings::instance().NAUGHTY_MODE);

    QJsonObject command_line_test_obj;
    command_line_test_obj.insert("RUN", QJsonValue(COMMAND_LINE_TEST_MODE));
    command_line_test_obj.insert("FOLDER", QJsonValue(QString::fromStdString(COMMAND_LINE_TEST_FOLDER)));

    {
        QJsonArray test_list;
        for(const auto& name : COMMAND_LINE_TEST_LIST){
            test_list.append(QJsonValue(name.c_str()));
        }
        command_line_test_obj.insert("TEST_LIST", test_list);
    }

    {
        QJsonArray ignore_list;
        for(const auto& name : COMMAND_LINE_IGNORE_LIST){
            ignore_list.append(QJsonValue(name.c_str()));
        }
        command_line_test_obj.insert("IGNORE_LIST", ignore_list);
    }

    obj.insert("COMMAND_LINE_TESTS", command_line_test_obj);

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
