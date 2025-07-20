/*  Resolution Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "ResolutionOption.h"

namespace PokemonAutomation{


ResolutionOption::ResolutionOption(
    std::string label, std::string description,
    int default_width, int default_height,
    int initial_x_pos, int initial_y_pos
)
    : GroupOption(std::move(label), LockMode::LOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , WIDTH("<b>Width:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_width(default_width))
    , HEIGHT("<b>Height:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_height(default_height))
    , X_POS("<b>X-position:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_width(initial_x_pos))
    , Y_POS("<b>Y-position:</b>", LockMode::LOCK_WHILE_RUNNING, scale_dpi_height(initial_y_pos))
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
    PA_ADD_OPTION(X_POS);
    PA_ADD_OPTION(Y_POS);
}


}
