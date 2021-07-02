/*  Global Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/String.h"
#include "CommonFramework/PersistentSettings.h"
#include "GlobalSettingsPanel.h"

namespace PokemonAutomation{


PanelDescriptorWrapper<GlobalSettings_Descriptor, GlobalSettings> GlobalSettings_Descriptor::INSTANCE;

GlobalSettings_Descriptor::GlobalSettings_Descriptor()
    : PanelDescriptor(
        QColor(),
        "GlobalSettings",
        "Global Settings",
        "",
        "Global Settings"
    )
{}


GlobalSettings::GlobalSettings(const GlobalSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
{
    m_options.emplace_back(
        "StatsFile",
        new String(
            PERSISTENT_SETTINGS().stats_file,
            "<b>Stats File:</b><br>Use the stats file here. Multiple instances of the program can use the same file.",
            "PA-Stats.txt"
        )
    );
    m_options.emplace_back(
        "Window Size (Width)",
        new SimpleInteger<uint32_t>(
            PERSISTENT_SETTINGS().window_width,
            "<b>Window Size (Width):</b><br>"
            "Set the width of the window. Restart application to take effect.<br>"
            "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
            "Note that the actual resolution will be subject to your monitor's DPI scaling.",
            1280
        )
    );
    m_options.emplace_back(
        "Window Size (height)",
        new SimpleInteger<uint32_t>(
            PERSISTENT_SETTINGS().window_height,
            "<b>Window Size (Height):</b><br>"
            "Set the height of the window. Restart application to take effect.<br>"
            "Use this to easily set the window to a specific resolution for streaming alignment.<br>"
            "Note that the actual resolution will be subject to your monitor's DPI scaling.",
            720
        )
    );
    m_options.emplace_back(
        "LogEverything",
        new BooleanCheckBox(
            PERSISTENT_SETTINGS().log_everything,
            "<b>Log Everything:</b><br>Log everything to the output window and output log. Will be very spammy.",
            false
        )
    );
    m_options.emplace_back(
        "DeveloperMode",
        new BooleanCheckBox(
            PERSISTENT_SETTINGS().developer_mode,
            "<b>Developer Mode:</b><br>Enable developer options. Restart application to take full effect.",
            false
        )
    );

    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(
            "DISCORD_WEBHOOK_ID",
            new String(
                PERSISTENT_SETTINGS().DISCORD_WEBHOOK_ID,
                "<b>Discord webhook ID:</b><br>Some programs can send discord messages in your own private server. Set this to your discord webhook ID.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_WEBHOOK_TOKEN",
            new String(
                PERSISTENT_SETTINGS().DISCORD_WEBHOOK_TOKEN,
                "<b>Discord webhook token:</b><br>Some programs can send discord messages in your own private server. Set this to your discord webhook token.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_USER_ID",
            new String(
                PERSISTENT_SETTINGS().DISCORD_USER_ID,
                "<b>Discord user ID:</b><br>Some programs can send discord messages in your own private server. Set this to your discord user ID.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_USER_SHORT_NAME",
            new String(
                PERSISTENT_SETTINGS().DISCORD_USER_SHORT_NAME,
                "<b>Discord user short name:</b><br>Some programs can send discord messages in your own private server. Set this to your discord user short name.",
                ""
            )
        );
    }
}


}
