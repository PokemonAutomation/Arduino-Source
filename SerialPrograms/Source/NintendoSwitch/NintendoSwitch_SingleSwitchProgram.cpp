/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch_SingleSwitchProgram.h"
#include "Framework/NintendoSwitch_SingleSwitchProgramOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramDescriptor::SingleSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass color_class,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
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
{}
std::unique_ptr<PanelInstance> SingleSwitchProgramDescriptor::make_panel() const{
    return std::make_unique<SingleSwitchProgramOption>(*this);
}



SingleSwitchProgramInstance::~SingleSwitchProgramInstance() = default;
SingleSwitchProgramInstance::SingleSwitchProgramInstance(
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
        ImageAttachmentMode::JPG,
        error_notification_tags

    )
    , NOTIFICATION_ERROR_FATAL(
        "Program Error (Fatal)",
        true, true,
        ImageAttachmentMode::JPG,
        error_notification_tags
    )
{}


void SingleSwitchProgramInstance::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    ProControllerContext context(scope, env.console.controller<ProController>());
    program(env, context);
}
void SingleSwitchProgramInstance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Not implemented.");
}


void SingleSwitchProgramInstance::start_program_controller_check(
    ControllerSession& session
){
    if (!session.ready()){
        throw UserSetupError(session.logger(), "Cannot Start: Controller is not ready.");
    }
}
void SingleSwitchProgramInstance::start_program_feedback_check(
    VideoStream& stream,
    FeedbackType feedback_type
){
    StartProgramChecks::check_feedback(stream, feedback_type);
}
void SingleSwitchProgramInstance::start_program_border_check(
    VideoStream& stream,
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


void SingleSwitchProgramInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
void SingleSwitchProgramInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue SingleSwitchProgramInstance::to_json() const{
    return m_options.to_json();
}
std::string SingleSwitchProgramInstance::check_validity() const{
    return m_options.check_validity();
}
void SingleSwitchProgramInstance::restore_defaults(){
    return m_options.restore_defaults();
}




}
}
