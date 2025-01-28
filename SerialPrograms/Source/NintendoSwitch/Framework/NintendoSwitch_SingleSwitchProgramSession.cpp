/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/SleepSuppressOption.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch_SingleSwitchProgramOption.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"
#include "Pokemon/Pokemon_Strings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramSession::SingleSwitchProgramSession(SingleSwitchProgramOption& option, size_t console_number)
    : ProgramSession(option.descriptor())
    , m_option(option)
    , m_system(option.system(), instance_id(), console_number)
    , m_scope(nullptr)
{}

SingleSwitchProgramSession::~SingleSwitchProgramSession(){
    SingleSwitchProgramSession::internal_stop_program();
    join_program_thread();
}


void SingleSwitchProgramSession::restore_defaults(){
    std::lock_guard<std::mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_option.restore_defaults();
}
std::string SingleSwitchProgramSession::check_validity() const{
    return m_option.check_validity();
}



void SingleSwitchProgramSession::run_program_instance(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    {
        std::lock_guard<std::mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    //  Startup Checks
    m_option.instance().start_program_controller_check(scope, m_system.controller_session());
    m_option.instance().start_program_feedback_check(scope, env.console, m_option.descriptor().feedback());
    if (m_option.descriptor().category() != Pokemon::STRING_POKEMON + " RSE"
        && m_option.descriptor().category() != Pokemon::STRING_POKEMON + " FRLG"){
        m_option.instance().start_program_border_check(scope, env.console);
    }

    m_scope.store(&scope, std::memory_order_release);

    try{
        SwitchControllerContext context(scope, env.console.controller());
        m_option.instance().program(env, context);
        context.wait_for_all_requests();
    }catch (...){
        m_scope.store(nullptr, std::memory_order_release);
        throw;
    }
    m_scope.store(nullptr, std::memory_order_release);
}
void SingleSwitchProgramSession::internal_stop_program(){
    WriteSpinLock lg(m_lock);
//    m_system.serial_session().stop();
    CancellableScope* scope = m_scope.load(std::memory_order_acquire);
    if (scope != nullptr){
        scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
    }

    //  Wait for program thread to finish.
    while (m_scope.load(std::memory_order_acquire) != nullptr){
        pause();
    }

//    m_system.serial_session().reset();
}
void SingleSwitchProgramSession::internal_run_program(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread();
    m_option.options().reset_state();

    if (!m_system.controller_session().ready()){
        report_error("Cannot Start: The controller is not ready.");
        return;
    }

    SleepSuppressScope sleep_scope(GlobalSettings::instance().SLEEP_SUPPRESS->PROGRAM_RUNNING);

    ProgramInfo program_info(
        identifier(),
        m_option.descriptor().category(),
        m_option.descriptor().display_name(),
        timestamp()
    );
    CancellableHolder<CancellableScope> scope;
    ControllerConnection& connection = m_system.controller_session().connection();
    SwitchController& switch_controller = dynamic_cast<SwitchController&>(connection);
    SingleSwitchProgramEnvironment env(
        program_info,
        scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        m_system.logger(),
        switch_controller,
        m_system.video(),
        m_system.overlay(),
        m_system.audio(),
        m_system.stream_history()
    );

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        run_program_instance(env, scope);
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        e.send_notification(env, m_option.instance().NOTIFICATION_PROGRAM_FINISH);
    }catch (InvalidConnectionStateException&){
    }catch (ScreenshotException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        e.add_stream_if_needed(env.console);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_notification(env, m_option.instance().NOTIFICATION_ERROR_FATAL);
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
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
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            env, m_option.instance().NOTIFICATION_ERROR_FATAL,
            "Unknown error."
        );
    }
}



}
}
