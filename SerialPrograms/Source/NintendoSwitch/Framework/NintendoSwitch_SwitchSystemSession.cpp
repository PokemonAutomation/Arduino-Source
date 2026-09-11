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
    bool allow_commands_while_locked,
    size_t console_number,
    std::optional<uint64_t> program_id
)
    : UiState<SwitchSystemSession, GameConsole::ConsoleSystemSession>(
        global_logger_raw(),
        option,
        allow_commands_while_locked,
        console_number,
        std::nullopt
    )
    , m_option(option)
{
    m_console_id = ProgramTracker::instance().add_console(program_id, *this);
}



JsonValue SwitchSystemSession::to_json() const{
    return m_option.to_json();
}
void SwitchSystemSession::load_json(const JsonValue& json){
    ConsoleSystemSession::load_json(json);
    m_option.load_json_self(json);
}




}
}
