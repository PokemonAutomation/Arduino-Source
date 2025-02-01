/*  Resolution Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Windows/DpiScaler.h"
#include "ResolutionOption.h"

namespace PokemonAutomation{


ResolutionOption::ResolutionOption(
    std::string label, std::string description,
    int default_width, int default_height
)
    : GroupOption(std::move(label), LockMode::LOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , WIDTH("<b>Width:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_width(default_width))
    , HEIGHT("<b>Height:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_height(default_height))
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
}


}
