/*  Framework Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/String.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "FrameworkSettingsPanel.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


FrameworkSettings::FrameworkSettings()
    : SettingsPanel(
        QColor(),
        "Framework Settings",
        "",
        "Global Framework Settings"
    )
{
    m_options.emplace_back(
        "CONNECT_CONTROLLER_DELAY",
        new TimeExpression<uint16_t>(
            CONNECT_CONTROLLER_DELAY,
            "<b>Connection Controller Delay:</b><br>Wait this long before starting the program. The LEDs normally flash during this time.",
            "5 * TICKS_PER_SECOND"
        )
    );
    m_options.emplace_back(
        "SETTINGS_TO_HOME_DELAY",
        new TimeExpression<uint16_t>(
            SETTINGS_TO_HOME_DELAY,
            "<b>Settings to Home Delay:</b><br>Delay from pressing home anywhere in the settings to return to the home menu.",
            "120"
        )
    );
    m_options.emplace_back(
        "START_GAME_REQUIRES_INTERNET",
        new BooleanCheckBox(
            START_GAME_REQUIRES_INTERNET,
            "<b>Start Game Requires Internet:</b><br>Set this to true if starting the game requires checking the internet. Otherwise, programs that require soft-resetting may not work properly.",
            false
        )
    );
    m_options.emplace_back(
        "TOLERATE_SYSTEM_UPDATE_MENU_FAST",
        new BooleanCheckBox(
            TOLERATE_SYSTEM_UPDATE_MENU_FAST,
            "<b>Tolerate System Update Menu (fast):</b><br>Some programs can bypass the system update menu at little performance cost. Setting this to true enables this.",
            true
        )
    );
    m_options.emplace_back(
        "TOLERATE_SYSTEM_UPDATE_MENU_SLOW",
        new BooleanCheckBox(
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW,
            "<b>Tolerate System Update Menu (slow):</b><br>Some programs can bypass the system update menu, but will take a noticeable performance hit. Setting this to true enables this.",
            false
        )
    );
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(
            "DISCORD_WEBHOOK_ID",
            new String(
                "<b>Discord webhook ID:</b><br>Some programs can send discord messages in your own private server. Set this to your discord webhook ID.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_WEBHOOK_TOKEN",
            new String(
                "<b>Discord webhook token:</b><br>Some programs can send discord messages in your own private server. Set this to your discord webhook token.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_USER_ID",
            new String(
                "<b>Discord user ID:</b><br>Some programs can send discord messages in your own private server. Set this to your discord user ID.",
                ""
            )
        );
        m_options.emplace_back(
            "DISCORD_USER_SHORT_NAME",
            new String(
                "<b>Discord user short name:</b><br>Some programs can send discord messages in your own private server. Set this to your discord user short name.",
                ""
            )
        );
    }
}
FrameworkSettings::FrameworkSettings(const QJsonValue& json)
    : FrameworkSettings()
{
    from_json(json);
}



}
}






