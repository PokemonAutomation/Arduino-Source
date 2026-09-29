/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch_MultiSwitchProgram.h"
#include "Framework/NintendoSwitch_MultiSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    std::vector<std::unique_ptr<GameConsole::ConsoleHandle>> consoles
)
    : GameConsole::MultiConsoleProgramEnvironment(
        program_info,
        scope,
        session,
        current_stats,
        historical_stats,
        std::move(consoles)
    )
{}



MultiSwitchProgramDescriptor::MultiSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass controller_class,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    size_t min_switches,
    size_t max_switches,
    size_t default_switches,
    PanelDeprecation deprecation,
    std::vector<std::string> required_resources
)
    : GameConsole::MultiConsoleProgramDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        controller_class,
        GameConsole::pick_color(controller_class),
        feedback,
        allow_commands_while_running,
        min_switches,
        max_switches,
        default_switches,
        deprecation,
        std::move(required_resources)
    )
{}
std::unique_ptr<PanelSession> MultiSwitchProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new MultiSwitchProgramSession(*this));
}






void MultiSwitchProgramInstance::program(GameConsole::MultiConsoleProgramEnvironment& env, CancellableScope& scope){
    program(static_cast<MultiSwitchProgramEnvironment&>(env), scope);
}
void MultiSwitchProgramInstance::run_start_program_checks(
    const ProgramDescriptor& descriptor,
    ProgramEnvironment& env
){
    const GameConsole::MultiConsoleProgramDescriptor& ldescriptor = dynamic_cast<const GameConsole::MultiConsoleProgramDescriptor&>(descriptor);
    GameConsole::MultiConsoleProgramEnvironment& lenv = dynamic_cast<GameConsole::MultiConsoleProgramEnvironment&>(env);
    size_t consoles = lenv.consoles();
    for (size_t c = 0; c < consoles; c++){
        start_program_feedback_check(ldescriptor, c, lenv.console(c));
        start_program_border_check(lenv.console(c), c, ldescriptor.feedback());
    }
}
void MultiSwitchProgramInstance::start_program_border_check(
    VideoStream& stream, size_t console_index,
    FeedbackType feedback_type
){
    switch (feedback_type){
    case FeedbackType::NONE:
        return;
    case FeedbackType::OPTIONAL_:
    case FeedbackType::REQUIRED:
    case FeedbackType::VIDEO_AUDIO:
        StartProgramChecks::check_border(stream);
    }
}










}
}
