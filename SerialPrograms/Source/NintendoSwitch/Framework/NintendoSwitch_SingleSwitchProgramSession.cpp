/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramSession::SingleSwitchProgramSession(const SingleSwitchProgramDescriptor& descriptor)
    : UiState<SingleSwitchProgramSession, GameConsole::ConsoleProgramSession>(
        descriptor,
        std::make_unique<ConsoleModelCell>()
    )
    , m_descriptor(descriptor)
{}


SingleSwitchProgramSession::~SingleSwitchProgramSession(){
    blocking_shutdown(
        logger(),
        "SingleSwitchProgramSession",
        [this]{ return try_shutdown(); }
    );
}




std::unique_ptr<ProgramEnvironment> SingleSwitchProgramSession::make_env(const ProgramInfo& program_info){
    auto env = std::make_unique<SingleSwitchProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(),
        historical_stats_tracker(),
        system()
    );

    ConsoleModelCell& console_type = static_cast<ConsoleModelCell&>(*system().extra_option());

    if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
        env->console.state().set_console_type(system().logger(), console_type);
    }else{
        env->console.state().set_console_type_user(console_type);
    }
    return env;
}










}
}
