/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Controllers/ControllerSession.h"
#include "Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Framework/NintendoSwitch_SingleSwitchProgramSession.h"
#include "NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void SingleSwitchProgramEnvironment::log_to_ui(const std::string& msg, Color color){
    console.overlay().add_log(msg, color);
}


SingleSwitchProgramDescriptor::SingleSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass color_class,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    PanelDeprecation deprecation,
    std::vector<std::string> required_resources
)
    : ProgramDescriptor(
        pick_color(color_class),
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation,
        true,
        std::move(required_resources)
    )
    , m_color_class(color_class)
    , m_feedback(feedback)
    , m_allow_commands_while_running(allow_commands_while_running == AllowCommandsWhenRunning::ENABLE_COMMANDS)
{}
std::unique_ptr<PanelSession> SingleSwitchProgramDescriptor::make_panel() const{
    return std::make_unique<SingleSwitchProgramSession>(*this);
}




void SingleSwitchProgramInstance::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    ProControllerContext context(scope, env.console.controller<ProController>());

    auto record_debug_video = [&](){
        if (GlobalSettings::instance().SAVE_DEBUG_VIDEOS_ON_SWITCH){
            context.controller().cancel_all_commands();
            env.log("Saving debug video on Switch...");
            env.console.overlay().add_log("Save Debug Video on Switch");
            pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            context.wait_for_all_requests();
        }
    };

    try{
        program(env, context);
    }catch (FatalProgramException&){
        record_debug_video();
        throw;
    }catch (OperationFailedException&){
        record_debug_video();
        throw;
    }
}
void SingleSwitchProgramInstance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Not implemented.");
}


void SingleSwitchProgramInstance::start_program_controller_check(
    SwitchSystemSession& session
){
    if (session.controllers() == 0){
        return;
    }
    if (!session.controller(0).ready()){
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





}
}
