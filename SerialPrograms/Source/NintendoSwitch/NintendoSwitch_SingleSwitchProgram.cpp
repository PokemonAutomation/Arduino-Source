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
#include "Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Framework/NintendoSwitch_SingleSwitchProgramSession.h"
#include "NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramEnvironment::SingleSwitchProgramEnvironment(
    const ProgramInfo& program_info,
    CancellableScope& scope,
    ProgramSession& session,
    StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    GameConsole::ConsoleSystemSession& system
)
    : GameConsole::ConsoleProgramEnvironment(
        program_info,
        scope,
        session,
        current_stats,
        historical_stats,
        std::make_unique<ConsoleHandle>(system)
    )
    , console(static_cast<ConsoleHandle&>(ConsoleProgramEnvironment::console()))
{}

SingleSwitchProgramDescriptor::SingleSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    ProgramControllerClass controller_class,
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    PanelDeprecation deprecation,
    std::vector<std::string> required_resources
)
    : GameConsole::ConsoleProgramDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        GameConsole::pick_color(controller_class),
        feedback,
        allow_commands_while_running,
        deprecation,
        std::move(required_resources)
    )
    , m_controller_class(controller_class)
{}
std::unique_ptr<PanelSession> SingleSwitchProgramDescriptor::make_panel() const{
    return std::make_unique<SingleSwitchProgramSession>(*this);
}



void SingleSwitchProgramInstance::program(GameConsole::ConsoleProgramEnvironment& env, CancellableScope& scope){
    program(static_cast<SingleSwitchProgramEnvironment&>(env), scope);
}
void SingleSwitchProgramInstance::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    ProControllerContext context(scope, env.console.controller<ProController>());
    if (!context->is_ready()){
        throw UserSetupError(context->logger(), "Controller is not ready.");
    }

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


void SingleSwitchProgramInstance::run_start_program_checks(
    const ProgramDescriptor& descriptor,
    ProgramEnvironment& env
){
    const GameConsole::ConsoleProgramDescriptor& ldescriptor = dynamic_cast<const GameConsole::ConsoleProgramDescriptor&>(descriptor);
    GameConsole::ConsoleProgramEnvironment& lenv = dynamic_cast<GameConsole::ConsoleProgramEnvironment&>(env);
    start_program_feedback_check(lenv.console(), ldescriptor.feedback());
    start_program_border_check(lenv.console(), ldescriptor.feedback());
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
