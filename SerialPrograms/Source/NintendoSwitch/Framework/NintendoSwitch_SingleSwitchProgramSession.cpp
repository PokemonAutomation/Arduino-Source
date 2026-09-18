/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/SleepSuppressOption.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
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
    , m_system_option(descriptor.allow_commands_while_running())
    , m_system(m_system_option, descriptor.allow_commands_while_running(), 0, instance_id())
    , m_instance(descriptor.make_instance(m_system))
{}


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



void SingleSwitchProgramSession::run_program_instance(SingleSwitchProgramEnvironment& env){
    {
        std::lock_guard<Mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    size_t controllers = env.console.controllers();

    //  Startup Checks
    if (controllers > 0){
        m_instance->start_program_controller_check(
            m_system.controller(0)
        );
    }
    m_instance->start_program_feedback_check(
        env.console,
        m_descriptor.feedback()
    );
    m_instance->start_program_border_check(
        env.console,
        m_descriptor.feedback()
    );

    //  Attach all the controllers to the scope so they can be cancelled from the top.
    FixedLimitVector<ControllerContext<AbstractController>> contexts(controllers);
    for (size_t c = 0; c < controllers; c++){
        contexts.emplace_back(*m_scope, env.console.controller(c));
    }

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }

    ScopeExit on_exit([&]{
        env.console.cancel_all_controllers();
    });

    m_instance->program(env, *m_scope);
    env.console.wait_for_all_controllers();
}
void SingleSwitchProgramSession::internal_run_program(const ProgramInfo& program_info){
    options().reset_state();

    SleepSuppressScope sleep_scope(GlobalSettings::instance().SLEEP_SUPPRESS->PROGRAM_RUNNING);

    SingleSwitchProgramEnvironment env(
        program_info,
        *m_scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        m_system
    );
    if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
        env.console.state().set_console_type(m_system.logger(), m_system.console_type());
    }else{
        env.console.state().set_console_type_user(m_system.console_type());
    }

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        env.log_to_ui("- Starting Program -");
        run_program_instance(static_cast<SingleSwitchProgramEnvironment&>(env));
        env.log_to_ui("- Program Finished -");
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
        env.log_to_ui("- Program Stopped -");
    }catch (ProgramCancelledException&){
        env.log_to_ui("- Program Stopped -");
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        env.log_to_ui("- Program Finished -");
        send_program_finished_notification(env, m_instance->NOTIFICATION_PROGRAM_FINISH, e.message(), *e.screenshot());
    }catch (InvalidConnectionStateException& e){
        logger().log("Program stopped due to connection issue.", COLOR_RED);
        env.log_to_ui("- Invalid Connection -", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
    }catch (OperationFailedExceptionWithScreenshot& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (OperationFailedException& e){ // no screenshot
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (FatalProgramException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Program Error -", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            message
        );
    }
    catch (std::exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Program Error -", COLOR_RED);
        std::string message = e.what();
        if (message.empty()){
            message = "Unknown std::exception.";
        }
        report_error(message);
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            message
        );
    }catch (...){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.log_to_ui("- Unknown Error -", COLOR_RED);
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            "Unknown error."
        );
    }
}










}
}
