/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



MultiSwitchProgramSession::MultiSwitchProgramSession(const MultiSwitchProgramDescriptor& descriptor)
    : GameConsole::MultiConsoleProgramSession(
        descriptor,
        std::make_unique<GameConsole::MultiConsoleSystemOption>(
            descriptor.min_consoles(),
            descriptor.max_consoles(),
            descriptor.default_consoles(),
            [](size_t console_index){
                return std::make_unique<GameConsole::ConsoleSystemOption>(
                    1, std::make_unique<ConsoleModelCell>()
                );
            }
        )
    )
    , m_descriptor(descriptor)
{}
MultiSwitchProgramSession::~MultiSwitchProgramSession(){
    blocking_shutdown(
        logger(),
        "MultiSwitchProgramSession",
        [this]{ return try_shutdown(); }
    );
}




std::unique_ptr<ProgramEnvironment> MultiSwitchProgramSession::make_env(const ProgramInfo& program_info){
    size_t consoles = system().active_consoles();
    std::vector<std::unique_ptr<GameConsole::ConsoleHandle>> handles;
    for (size_t c = 0; c < consoles; c++){
        GameConsole::ConsoleSystemSession& session = system()[c];
        auto uptr = std::make_unique<ConsoleHandle>(session);
        ConsoleHandle* handle = uptr.get();
        handles.emplace_back(std::move(uptr));

        ConsoleModelCell& console_type = static_cast<ConsoleModelCell&>(*session.extra_option());

        ConsoleState& state = handle->state();
        if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
            state.set_console_type(handle->logger(), console_type);
        }else{
            state.set_console_type_user(console_type);
        }
    }
    return std::make_unique<MultiSwitchProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        std::move(handles)
    );
}





}
}
