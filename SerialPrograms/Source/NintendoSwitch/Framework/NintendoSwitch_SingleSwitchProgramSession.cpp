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
#include "Common/Cpp/Concurrency/SpinPause.h"
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
    , m_system(m_system_option, 0, instance_id())
    , m_instance(descriptor.make_instance(m_system))
    , m_scope(nullptr)
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


void SingleSwitchProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
ConfigOption& SingleSwitchProgramSession::options(){
    return m_instance->m_options;
}
std::string SingleSwitchProgramSession::check_validity() const{
    return m_instance->check_validity();
}



void SingleSwitchProgramSession::run_program_instance(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    {
        std::lock_guard<Mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    //  Startup Checks
    m_instance->start_program_controller_check(
        m_system.controller()
    );
    m_instance->start_program_feedback_check(
        env.console,
        m_descriptor.feedback()
    );
    m_instance->start_program_border_check(
        env.console,
        m_descriptor.feedback()
    );

    //  Attach all the controllers to the scope so they can be cancelled from the top.
    size_t controllers = env.console.controllers();
    FixedLimitVector<ControllerContext<AbstractController>> contexts(controllers);
    for (size_t c = 0; c < controllers; c++){
        contexts.emplace_back(scope, env.console.controller(c));
    }

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
        m_scope.store(&scope, std::memory_order_release);
    }

    ScopeExit on_exit([&, this]{
        env.console.cancel_all_controllers();
        std::lock_guard<Mutex> lg(program_lock());
        m_scope.store(nullptr, std::memory_order_release);
    });

    m_instance->program(env, scope);
    env.console.wait_for_all_controllers();
}
void SingleSwitchProgramSession::internal_stop_program(){
    {
        std::lock_guard<Mutex> lg(program_lock());
        CancellableScope* scope = m_scope.load(std::memory_order_acquire);
        if (scope != nullptr){
            scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
        }
    }

    //  Wait for program thread to finish.
    while (m_scope.load(std::memory_order_acquire) != nullptr){
        pause();
    }
}
void SingleSwitchProgramSession::internal_run_program(){
    {
        CancellableHolder<CancellableScope> scope;
        {
            std::lock_guard<Mutex> lg(program_lock());
            if (current_state() != ProgramState::RUNNING){
                return;
            }
            m_scope.store(&scope, std::memory_order_release);
        }
        bool success = download_prereqs(scope);
        {
            std::lock_guard<Mutex> lg(program_lock());
            m_scope.store(nullptr, std::memory_order_release);
        }

        if (!success){
            return;
        }
    }

    m_instance->m_options.reset_state();

    SleepSuppressScope sleep_scope(GlobalSettings::instance().SLEEP_SUPPRESS->PROGRAM_RUNNING);

    ProgramInfo program_info(
        identifier(),
        m_descriptor.category(),
        m_descriptor.display_name(),
        timestamp()
    );
    CancellableHolder<CancellableScope> scope;
    SingleSwitchProgramEnvironment env(
        program_info,
        scope,
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
        env.console.overlay().clear_log();
        env.console.overlay().add_log("- Starting Program -");
        run_program_instance(env, scope);
        env.console.overlay().add_log("- Program Finished -");
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
        env.console.overlay().add_log("- Program Stopped -");
    }catch (ProgramCancelledException&){
        env.console.overlay().add_log("- Program Stopped -");
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        env.console.overlay().add_log("- Program Finished -");
        send_program_finished_notification(env, m_instance->NOTIFICATION_PROGRAM_FINISH, e.message(), *e.screenshot());
    }catch (InvalidConnectionStateException& e){
        logger().log("Program stopped due to connection issue.", COLOR_RED);
        env.console.overlay().add_log("- Invalid Connection -", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
    }catch (OperationFailedExceptionWithScreenshot& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.console.overlay().add_log("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (OperationFailedException& e){ // no screenshot
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.console.overlay().add_log("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (FatalProgramException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.console.overlay().add_log("- Program Error -", COLOR_RED);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.console.overlay().add_log("- Program Error -", COLOR_RED);
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
        env.console.overlay().add_log("- Program Error -", COLOR_RED);
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
        env.console.overlay().add_log("- Unknown Error -", COLOR_RED);
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            "Unknown error."
        );
    }
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










}
}
