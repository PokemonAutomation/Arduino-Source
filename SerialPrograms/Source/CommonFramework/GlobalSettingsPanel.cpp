/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/PersistentSettings.h"
#include "GlobalSettingsPanel.h"

namespace PokemonAutomation{


PanelDescriptorWrapper<GlobalSettings_Descriptor, GlobalSettings> GlobalSettings_Descriptor::INSTANCE;

GlobalSettings_Descriptor::GlobalSettings_Descriptor()
    : PanelDescriptor(
        QColor(),
        "",
        "Global Settings",
        "",
        "Global Settings"
    )
{}


GlobalSettings::GlobalSettings(const GlobalSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , STATS_FILE(
        PERSISTENT_SETTINGS().stats_file,
        "<b>Stats File:</b><br>Use the stats file here. Multiple instances of the program can use the same file.",
        "PA-Stats.txt"
    )
    , WINDOW_WIDTH(
        PERSISTENT_SETTINGS().window_width,
        "<b>Window Size (Width):</b><br>"
        "Set the width of the window. Restart application to take effect.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
        "Note that the actual resolution will be subject to your monitor's DPI scaling.",
        1280
    )
    , WINDOW_HEIGHT(
        PERSISTENT_SETTINGS().window_height,
        "<b>Window Size (Height):</b><br>"
        "Set the height of the window. Restart application to take effect.<br>"
        "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
        "Note that the actual resolution will be subject to your monitor's DPI scaling.",
        720
    )
    , LOG_EVERYTHING(
        PERSISTENT_SETTINGS().log_everything,
        "<b>Log Everything:</b><br>Log everything to the output window and output log. Will be very spammy.",
        false
    )
    , SAVE_DEBUG_IMAGES(
        PERSISTENT_SETTINGS().save_debug_images,
        "<b>Save Debug Images:</b><br>"
        "If the program fails to read something when it should succeed, save the image for debugging purposes.",
        true
    )
    , DEVELOPER_TOKEN(
        PERSISTENT_SETTINGS().developer_token,
        "<b>Developer Token:</b><br>Restart application to take full effect after changing this.",
        ""
    )
{
    PA_ADD_OPTION(STATS_FILE);
    PA_ADD_OPTION(WINDOW_WIDTH);
    PA_ADD_OPTION(WINDOW_HEIGHT);
    PA_ADD_OPTION(LOG_EVERYTHING);
    PA_ADD_OPTION(SAVE_DEBUG_IMAGES);
    add_option(PERSISTENT_SETTINGS().discord_settings, "DISCORD_SETTINGS");
    PA_ADD_OPTION(DEVELOPER_TOKEN);
}


}
