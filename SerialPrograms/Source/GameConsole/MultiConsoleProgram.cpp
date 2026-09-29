/*  Multi-Console Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "CommonFramework/Panels/PanelSession.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Framework/MultiConsoleProgramSession.h"
#include "MultiConsoleProgram.h"

namespace PokemonAutomation{
namespace GameConsole{



MultiConsoleProgramEnvironment::MultiConsoleProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    std::vector<std::unique_ptr<ConsoleHandle>> consoles
)
    : ProgramEnvironment(program_info, session, current_stats, historical_stats)
    , m_consoles(std::move(consoles))
{
    for (std::unique_ptr<ConsoleHandle>& console : m_consoles){
        console->initialize_inference_threads(scope);
    }
}
void MultiConsoleProgramEnvironment::log_to_ui(const std::string& msg, Color color){
    for (std::unique_ptr<ConsoleHandle>& console : m_consoles){
        console->overlay().add_log(msg, color);
    }
}

void MultiConsoleProgramEnvironment::run_in_parallel(
    CancellableScope& scope, size_t s, size_t e,
    const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
){
    GlobalThreadPools::unlimited_realtime().run_in_parallel(
        [&](size_t index){
            ConsoleHandle& console = *m_consoles[index];
            ThreadUtilizationStat stat(current_thread_handle(), "Program Thread " + std::to_string(index) + ":");
            console.overlay().add_stat(stat);
            ScopeExit on_exit([&]{
                console.overlay().remove_stat(stat);
            });
            func(scope, console);
            console.controller().wait_for_all(&scope);
        },
        s, e
    );
}




MultiConsoleProgramDescriptor::MultiConsoleProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass controller_class,
    Color color,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    size_t min_consoles,
    size_t max_consoles,
    size_t default_consoles,
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
    , m_min_consoles(min_consoles)
    , m_max_consoles(max_consoles)
    , m_default_consoles(default_consoles)
{}
std::unique_ptr<PanelSession> MultiConsoleProgramDescriptor::make_panel() const{
    return std::make_unique<MultiConsoleProgramSession>(*this);
}




void MultiConsoleProgramInstance::run_start_program_checks(
    const ProgramDescriptor& descriptor,
    ProgramEnvironment& env
){
    const MultiConsoleProgramDescriptor& ldescriptor = dynamic_cast<const MultiConsoleProgramDescriptor&>(descriptor);
    MultiConsoleProgramEnvironment& lenv = dynamic_cast<MultiConsoleProgramEnvironment&>(env);
    size_t consoles = lenv.consoles();
    for (size_t c = 0; c < consoles; c++){
        start_program_feedback_check(ldescriptor, c, lenv.console(c));
    }
}
void MultiConsoleProgramInstance::start_program_feedback_check(
    const MultiConsoleProgramDescriptor& descriptor,
    size_t console_index, VideoStream& stream
){
    StartProgramChecks::check_feedback(stream, descriptor.feedback());
}


































}
}
