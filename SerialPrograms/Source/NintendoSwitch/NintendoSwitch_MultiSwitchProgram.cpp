/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch_MultiSwitchProgram.h"
#include "Framework/NintendoSwitch_MultiSwitchProgramOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

MultiSwitchProgramEnvironment::~MultiSwitchProgramEnvironment(){}

MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    FixedLimitVector<ConsoleHandle> p_switches
)
    : ProgramEnvironment(program_info, session, current_stats, historical_stats)
    , consoles(std::move(p_switches))
{
    for (ConsoleHandle& console : consoles){
        console.initialize_inference_threads(scope, realtime_inference_dispatcher());
    }
}

void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope,
    const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
){
    run_in_parallel(scope, 0, consoles.size(), func);
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope,
    const std::function<void(ConsoleHandle& console, ProControllerContext& context)>& func
){
    run_in_parallel(scope, 0, consoles.size(), func);
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope, size_t s, size_t e,
    const std::function<void(CancellableScope& scope, ConsoleHandle& console)>& func
){
    realtime_dispatcher().run_in_parallel(
        s, e,
        [&](size_t index){
            ConsoleHandle& console = consoles[index];
            ThreadUtilizationStat stat(current_thread_handle(), "Program Thread " + std::to_string(index) + ":");
            console.overlay().add_stat(stat);
            try{
                func(scope, console);
                console.controller().wait_for_all(&scope);
                console.overlay().remove_stat(stat);
            }catch (...){
                console.overlay().remove_stat(stat);
                throw;
            }
        }
    );
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    CancellableScope& scope, size_t s, size_t e,
    const std::function<void(ConsoleHandle& console, ProControllerContext& context)>& func
){
    realtime_dispatcher().run_in_parallel(
        s, e,
        [&](size_t index){
            ConsoleHandle& console = consoles[index];
            ThreadUtilizationStat stat(current_thread_handle(), "Program Thread " + std::to_string(index) + ":");
            console.overlay().add_stat(stat);
            try{
                ProControllerContext context(scope, consoles[index].controller<ProController>());   //  REMOVE: don't use pro_controller()
                func(console, context);
                context.wait_for_all_requests();
                console.overlay().remove_stat(stat);
            }catch (...){
                console.overlay().remove_stat(stat);
                throw;
            }
        }
    );
}

void MultiSwitchProgramEnvironment::add_overlay_log_to_all_consoles(const std::string& message, Color color){
    for (auto&console: consoles){
        console.overlay().add_log(message, color);
    }
}

void MultiSwitchProgramEnvironment::clear_all_overlay_logs(){
    for (auto&console: consoles){
        console.overlay().clear_log();
    }
}


MultiSwitchProgramDescriptor::MultiSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass color_class,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    size_t min_switches,
    size_t max_switches,
    size_t default_switches,
    bool deprecated
)
    : ProgramDescriptor(
        pick_color(color_class),
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
    , m_color_class(color_class)
    , m_feedback(feedback)
    , m_allow_commands_while_running(allow_commands_while_running == AllowCommandsWhenRunning::ENABLE_COMMANDS)
    , m_deprecated(deprecated)
    , m_min_switches(min_switches)
    , m_max_switches(max_switches)
    , m_default_switches(default_switches)
{}
std::unique_ptr<PanelInstance> MultiSwitchProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelInstance>(new MultiSwitchProgramOption(*this));
}




MultiSwitchProgramInstance::~MultiSwitchProgramInstance() = default;
MultiSwitchProgramInstance::MultiSwitchProgramInstance(
    const std::vector<std::string>& error_notification_tags
)
    : m_options(LockMode::UNLOCK_WHILE_RUNNING)
    , NOTIFICATION_PROGRAM_FINISH(
        "Program Finished",
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_ERROR_RECOVERABLE(
        "Program Error (Recoverable)",
        true, false,
        ImageAttachmentMode::PNG,
        error_notification_tags
    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
        ImageAttachmentMode::PNG,
        error_notification_tags
    )
{}


void MultiSwitchProgramInstance::start_program_controller_check(
    ControllerSession& session, size_t console_index
){
    if (!session.ready()){
        throw UserSetupError(session.logger(), "Cannot Start: Controller is not ready.");
    }
}
void MultiSwitchProgramInstance::start_program_feedback_check(
    VideoStream& stream, size_t console_index,
    FeedbackType feedback_type
){
    StartProgramChecks::check_feedback(stream, feedback_type);
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


void MultiSwitchProgramInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
void MultiSwitchProgramInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue MultiSwitchProgramInstance::to_json() const{
    return m_options.to_json();
}
std::string MultiSwitchProgramInstance::check_validity() const{
    return m_options.check_validity();
}
void MultiSwitchProgramInstance::restore_defaults(){
    return m_options.restore_defaults();
}












}
}
