/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QCryptographicHash>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
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
    , WINDOW_WIDTH(
        "<b>Window Size (Width):</b><br>"
        "Set the width of the window. Restart application to take effect.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
        "Note that the actual resolution will be subject to your monitor's DPI scaling.",
        1280
    )
    , WINDOW_HEIGHT(
        "<b>Window Size (Height):</b><br>"
        "Set the height of the window. Restart application to take effect.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
        "Note that the actual resolution will be subject to your monitor's DPI scaling.",
        720
    )
    , m_discord_settings(
        "<font size=4><b>Discord Settings:</b> Integrate with Discord. "
//        "You can also control the program from Discord. "
        "(<a href=\"" + ONLINE_DOC_URL + "ComputerControl/blob/master/Wiki/Software/DiscordIntegration.md" +
        "\">online documentation</a>)</font></font>"
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
    , NAUGHTY_MODE("<b>Naughty Mode:</b>", false)
    , DEVELOPER_TOKEN(
        true,
        "<b>Developer Token:</b><br>Restart application to take full effect after changing this.",
        "", ""
    )
{
    PA_ADD_OPTION(SEND_ERROR_REPORTS);
    PA_ADD_OPTION(STATS_FILE);
    PA_ADD_OPTION(WINDOW_WIDTH);
    PA_ADD_OPTION(WINDOW_HEIGHT);
    PA_ADD_STATIC(m_discord_settings);
    PA_ADD_OPTION(DISCORD);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(LOG_EVERYTHING);
    PA_ADD_OPTION(SAVE_DEBUG_IMAGES);
//    PA_ADD_OPTION(NAUGHTY_MODE);
    PA_ADD_OPTION(DEVELOPER_TOKEN);
}

void GlobalSettings::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();

    //  Naughty mode.
    NAUGHTY_MODE.load_json(json_get_value_nothrow(obj, "NAUGHTY_MODE"));

    //  Developer mode stuff.
    DEVELOPER_TOKEN.load_json(json_get_value_nothrow(obj, "DEVELOPER_TOKEN"));
    {
        std::string token = DEVELOPER_TOKEN.get().toStdString();
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
        hash.addData(token.c_str(), (int)token.size());
        DEVELOPER_MODE = TOKENS.find(hash.result().toHex().toStdString()) != TOKENS.end();
    }
//    if (DEVELOPER_MODE){
        DISCORD.enable_integration();
//    }

    BatchOption::load_json(json);
}
QJsonValue GlobalSettings::to_json() const{
    QJsonObject obj = BatchOption::to_json().toObject();
    obj.insert("NAUGHTY_MODE", NAUGHTY_MODE.to_json());
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
