/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramSession::SingleSwitchProgramSession(const SingleSwitchProgramDescriptor& descriptor)
    : UiState<SingleSwitchProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(1, std::make_unique<ConsoleModelCell>())
    , m_system(m_system_option, descriptor.allow_commands_while_running(), 0, instance_id())
{
    m_instance = descriptor.make_instance(m_system);
}


bool SingleSwitchProgramSession::try_shutdown(){
    SingleSwitchProgramSession::internal_stop_program();
    join_program_thread();
    return m_system.try_shutdown();
}
SingleSwitchProgramSession::~SingleSwitchProgramSession(){
    blocking_shutdown(
        logger(),
        "SingleSwitchProgramSession",
        [this]{ return try_shutdown(); }
    );
}


ConfigOption& SingleSwitchProgramSession::options(){
    return m_instance->options();
}


std::string SingleSwitchProgramSession::check_validity() const{
    return m_instance->check_validity();
}
void SingleSwitchProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue SingleSwitchProgramSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["SwitchSetup"] = m_system_option.to_json();
    return obj;
}
void SingleSwitchProgramSession::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("SwitchSetup");
    if (value){
        m_system.load_json(*value);
    }
    m_instance->load_json(json);
}



std::unique_ptr<ProgramEnvironment> SingleSwitchProgramSession::make_env(const ProgramInfo& program_info){
    auto env = std::make_unique<SingleSwitchProgramEnvironment>(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        m_system
    );

    ConsoleModelCell& console_type = static_cast<ConsoleModelCell&>(*m_system.extra_option());

    if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
        env->console.state().set_console_type(m_system.logger(), console_type);
    }else{
        env->console.state().set_console_type_user(console_type);
    }
    return env;
}
void SingleSwitchProgramSession::internal_run_program(ProgramEnvironment& env){
    SingleSwitchProgramInstance& instance = static_cast<SingleSwitchProgramInstance&>(*m_instance);
    SingleSwitchProgramEnvironment& lenv = static_cast<SingleSwitchProgramEnvironment&>(env);

    size_t controllers = lenv.console.controllers();

    //  Startup Checks
    instance.start_program_feedback_check(
        lenv.console,
        m_descriptor.feedback()
    );
    instance.start_program_border_check(
        lenv.console,
        m_descriptor.feedback()
    );

    //  Attach all the controllers to the scope so they can be cancelled from the top.
    FixedLimitVector<ControllerContext<AbstractController>> contexts(controllers);
    for (size_t c = 0; c < controllers; c++){
        contexts.emplace_back(*m_scope, lenv.console.controller(c));
    }

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }

    ScopeExit on_exit([&]{
        lenv.console.cancel_all_controllers();
    });

    instance.program(lenv, *m_scope);
    lenv.console.wait_for_all_controllers();
}










}
}
