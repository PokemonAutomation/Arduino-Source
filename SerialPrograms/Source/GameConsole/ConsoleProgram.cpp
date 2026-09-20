/*  Console Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Framework/ConsoleSystemSession.h"
#include "ConsoleProgram.h"

namespace PokemonAutomation{
namespace GameConsole{



ConsoleProgramEnvironment::ConsoleProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    ConsoleSystemSession& system
)
    : ProgramEnvironment(program_info, session, current_stats, historical_stats)
    , m_console(new ConsoleHandle(system))
{
    m_console->initialize_inference_threads(scope);
}
ConsoleProgramEnvironment::ConsoleProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    std::unique_ptr<ConsoleHandle> console
)
    : ProgramEnvironment(program_info, session, current_stats, historical_stats)
    , m_console(std::move(console))
{
    m_console->initialize_inference_threads(scope);
}
void ConsoleProgramEnvironment::log_to_ui(const std::string& msg, Color color){
    m_console->overlay().add_log(msg, color);
}
std::unique_ptr<PanelSession> ConsoleProgramDescriptor::make_panel() const{
    //  TODO
    return nullptr;
}



void ConsoleProgramInstance::start_program_controller_check(
    ConsoleSystemSession& session
){
    if (session.controllers() == 0){
        return;
    }
    if (!session.controller(0).ready()){
        throw UserSetupError(session.logger(), "Cannot Start: Controller is not ready.");
    }
}
void ConsoleProgramInstance::start_program_feedback_check(
    VideoStream& stream,
    FeedbackType feedback_type
){
    StartProgramChecks::check_feedback(stream, feedback_type);
}



}
}
