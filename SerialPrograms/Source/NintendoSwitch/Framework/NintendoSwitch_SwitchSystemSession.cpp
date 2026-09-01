/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Integrations/ProgramTracker.h"
#include "NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch_SwitchSystemSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



bool SwitchSystemSession::try_shutdown() noexcept{
    ProgramTracker::instance().remove_console(m_console_id);
    return ConsoleSystemSession::try_shutdown();
}
SwitchSystemSession::~SwitchSystemSession(){
    blocking_shutdown(
        logger(),
        "SwitchSystemSession",
        [this]{ return try_shutdown(); }
    );
}
SwitchSystemSession::SwitchSystemSession(
    SwitchSystemOption& option,
    uint64_t program_id,
    size_t console_number
)
    : ConsoleSystemSession(global_logger_raw(), option, console_number)
    , m_option(option)
{
    m_console_id = ProgramTracker::instance().add_console(program_id, *this);
}





}
}
