/*  Console Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Panels/PanelSession.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Framework/ConsoleSystemSession.h"
#include "Framework/ConsoleProgramSession.h"
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


ConsoleProgramDescriptor::ConsoleProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass controller_class,
    Color color,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    PanelDeprecation deprecation,
    std::vector<std::string> required_resources
)
    : ProgramDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        color,
        deprecation,
        true,
        std::move(required_resources)
    )
    , m_controller_class(controller_class)
    , m_feedback(feedback)
    , m_allow_commands_while_running(
        allow_commands_while_running == AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}
std::unique_ptr<PanelSession> ConsoleProgramDescriptor::make_panel() const{
    return std::make_unique<ConsoleProgramSession>(*this);
}



void ConsoleProgramInstance::run_start_program_checks(
    const ProgramDescriptor& descriptor,
    ProgramEnvironment& env
){
    const ConsoleProgramDescriptor& ldescriptor = dynamic_cast<const ConsoleProgramDescriptor&>(descriptor);
    ConsoleProgramEnvironment& lenv = dynamic_cast<ConsoleProgramEnvironment&>(env);
    start_program_feedback_check(ldescriptor, lenv.console());
}
void ConsoleProgramInstance::start_program_feedback_check(
    const ConsoleProgramDescriptor& descriptor,
    VideoStream& stream
){
    StartProgramChecks::check_feedback(stream, descriptor.feedback());
}



}
}
