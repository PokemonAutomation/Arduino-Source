/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Options/Environment/SleepSuppressOption.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_MultiSwitchProgramOption.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void MultiSwitchProgramSession::add_listener(Listener& listener){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_listeners.add(listener);
}
void MultiSwitchProgramSession::remove_listener(Listener& listener){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}




MultiSwitchProgramSession::MultiSwitchProgramSession(MultiSwitchProgramOption& option)
    : ProgramSession(option.descriptor())
    , m_option(option)
    , m_system(option.system(), instance_id())
    , m_scope(nullptr)
    , m_sanitizer("MultiSwitchProgramSession")
{
//    WriteSpinLock lg(m_lock);
    m_option.instance().update_active_consoles(option.system().count());
    m_system.add_listener(*this);
}

MultiSwitchProgramSession::~MultiSwitchProgramSession(){
    MultiSwitchProgramSession::internal_stop_program();
    m_system.remove_listener(*this);
    join_program_thread();
}



void MultiSwitchProgramSession::restore_defaults(){
    auto ScopeCheck = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_option.restore_defaults();
}
std::string MultiSwitchProgramSession::check_validity() const{
    auto ScopeCheck = m_sanitizer.check_scope();
    return m_option.check_validity();
}



void MultiSwitchProgramSession::run_program_instance(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    auto ScopeCheck = m_sanitizer.check_scope();
    {
        std::lock_guard<std::mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    //  Startup Checks
    size_t consoles = m_system.count();
    for (size_t c = 0; c < consoles; c++){
        m_option.instance().start_program_controller_check(
            m_system[c].controller_session(), c
        );
        m_option.instance().start_program_feedback_check(
            env.consoles[c], c,
            m_option.descriptor().feedback()
        );
        m_option.instance().start_program_border_check(
            env.consoles[c], c,
            m_option.descriptor().feedback()
        );
    }

    {
        std::lock_guard<std::mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
        m_scope.store(&scope, std::memory_order_release);
    }

    try{
        m_option.instance().program(env, scope);
        for (size_t c = 0; c < consoles; c++){
            ControllerContext<AbstractController> context(scope, env.consoles[c].controller());
            context.wait_for_all_requests();
        }
    }catch (...){
        for (size_t c = 0; c < consoles; c++){
            try{
                env.consoles[c].controller().cancel_all_commands();
            }catch (...){}
        }
        std::lock_guard<std::mutex> lg(program_lock());
        m_scope.store(nullptr, std::memory_order_release);
        throw;
    }

    std::lock_guard<std::mutex> lg(program_lock());
    m_scope.store(nullptr, std::memory_order_release);
}
void MultiSwitchProgramSession::internal_stop_program(){
    auto ScopeCheck = m_sanitizer.check_scope();
    {
        std::lock_guard<std::mutex> lg(program_lock());
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
void MultiSwitchProgramSession::internal_run_program(){
    auto ScopeCheck = m_sanitizer.check_scope();
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(logger());
    m_option.options().reset_state();

    SleepSuppressScope sleep_scope(GlobalSettings::instance().SLEEP_SUPPRESS->PROGRAM_RUNNING);

    //  Lock the system to prevent the # of Switches from changing.
    std::lock_guard<MultiSwitchSystemSession> lg(m_system);

    ProgramInfo program_info(
        identifier(),
        m_option.descriptor().category(),
        m_option.descriptor().display_name(),
        timestamp()
    );

    size_t consoles = m_system.count();
    FixedLimitVector<ConsoleHandle> handles(consoles);
    for (size_t c = 0; c < consoles; c++){
        SwitchSystemSession& session = m_system[c];
        if (!session.controller_session().ready()){
            report_error("Cannot Start: The controller is not ready.");
            return;
        }
        AbstractController* controller = session.controller_session().controller();
        handles.emplace_back(
            c,
            session.logger(),
            *controller,
            session.video(),
            session.overlay(),
            session.audio(),
            session.stream_history()
        );

        ConsoleState& state = handles.back().state();
        if (ConsoleSettings::instance().TRUST_USER_CONSOLE_SELECTION){
            state.set_console_type(handles.back().logger(), session.console_type());
        }else{
            state.set_console_type_user(session.console_type());
        }
    }



    CancellableHolder<CancellableScope> scope;
    MultiSwitchProgramEnvironment env(
        program_info,
        scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        std::move(handles)
    );

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        env.add_overlay_log_to_all_consoles("- Starting Program -");
        run_program_instance(env, scope);
//        m_setup->wait_for_all_requests();
        env.add_overlay_log_to_all_consoles("- Program Finished -");
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException& e){
        logger().log("Program Stopped (OperationCancelledException): " + e.message(), COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Program Stopped -");
    }catch (ProgramCancelledException& e){
        logger().log("Program Stopped (ProgramCancelledException): " + e.message(), COLOR_BLUE);
        env.add_overlay_log_to_all_consoles("- Program Stopped -");
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        env.add_overlay_log_to_all_consoles("- Program Finished -");
        e.send_notification(env, m_option.instance().NOTIFICATION_PROGRAM_FINISH);
    }catch (InvalidConnectionStateException& e){
        logger().log("Program stopped due to connection issue.", COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Invalid Connection -", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
    }catch (ScreenshotException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Program Error -", COLOR_RED);
        //  If the exception doesn't already have console information,
        //  attach the 1st console here.
        e.add_stream_if_needed(env.consoles[0]);

        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_notification(env);
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Program Error -", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        send_program_fatal_error_notification(
            env, m_option.instance().NOTIFICATION_ERROR_FATAL,
            message
        );
    }catch (std::exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Program Error -", COLOR_RED);
        std::string message = e.what();
        if (message.empty()){
            message = "Unknown std::exception.";
        }
        report_error(message);
        send_program_fatal_error_notification(
            env, m_option.instance().NOTIFICATION_ERROR_FATAL,
            message
        );
    }catch (...){
        logger().log("Program stopped with an exception!", COLOR_RED);
        env.add_overlay_log_to_all_consoles("- Unknown Error -", COLOR_RED);
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            env, m_option.instance().NOTIFICATION_ERROR_FATAL,
            "Unknown error."
        );
    }
}


void MultiSwitchProgramSession::shutdown(){
    auto ScopeCheck = m_sanitizer.check_scope();
    internal_stop_program();
}
void MultiSwitchProgramSession::startup(size_t switch_count){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_option.instance().update_active_consoles(switch_count);
    m_listeners.run_method_unique(&Listener::redraw_options);
}






}
}
