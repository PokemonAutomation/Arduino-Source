/*  Fatal Program Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "FatalProgramException.h"

namespace PokemonAutomation{


FatalProgramException::FatalProgramException(ScreenshotException&& e)
    : ScreenshotException(
        e.m_send_error_report,
        std::move(e.m_message),
        std::move(e.m_screenshot)
    )
{}






}
