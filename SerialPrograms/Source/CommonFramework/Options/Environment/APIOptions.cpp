/*  API Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "APIOptions.h"
#include "CommonFramework/Panels/PanelTools.h"

namespace PokemonAutomation{

APIOptions::APIOptions()
    : GroupOption(
        "API",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , DESCRIPTION(
        "<font color=\"orange\">Note: The API is experimental and not all features are fully implemented.</font><br><br>"
        "<font color=\"red\">Warning: The API is not secure and should not be exposed to the internet.</font><br><br>"
        "After changing these settings, you must restart the program for them to take effect."
    )
    , ENABLE_API(
        "<b>Enable API:</b><br>"
        "Enable the HTTP API and WebSockets to control the program remotely.<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , HTTP_PORT(
        "<b>HTTP Port:</b><br>"
        "This can't be the same as the WebSocket Port<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        8080, 1025, 65535
    )
    , WS_PORT(
        "<b>WebSocket Port:</b><br>"
        "This can't be the same as the HTTP Port<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        8081, 1025, 65535
    )
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(ENABLE_API);
    PA_ADD_OPTION(HTTP_PORT);
    PA_ADD_OPTION(WS_PORT);
}

}
