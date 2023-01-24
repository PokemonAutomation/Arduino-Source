/*  Single Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_SingleSwitchProgramOption.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramSession::SingleSwitchProgramSession(SingleSwitchProgramOption& option, size_t console_number)
    : ProgramSession(option.descriptor())
    , m_option(option)
    , m_system(option.system(), instance_id(), console_number)
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

    if (!m_system.serial_session().is_ready()){
        throw UserSetupError(m_system.logger(), "Cannot Start: Serial connection not ready.");
    }

    start_program_video_check(env.console, m_option.descriptor().feedback());

    {
        SpinLockGuard lg(m_lock);
        m_scope = &scope;
    }

    try{
        BotBaseContext context(scope, env.console.botbase());
        m_option.instance().program(env, context);
    }catch (...){
        SpinLockGuard lg(m_lock);
        m_scope = nullptr;
        throw;
    }
    SpinLockGuard lg(m_lock);
    m_scope = nullptr;
}
void SingleSwitchProgramSession::internal_stop_program(){
    SpinLockGuard lg(m_lock);
    m_system.serial_session().stop();
    if (m_scope != nullptr){
        m_scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
    }
    m_system.serial_session().reset();
}
void SingleSwitchProgramSession::internal_run_program(){
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread();
    m_option.options().reset_state();

    ProgramInfo program_info(
        identifier(),
        m_option.descriptor().category(),
        m_option.descriptor().display_name(),
        timestamp()
    );
    CancellableHolder<CancellableScope> scope;
    SingleSwitchProgramEnvironment env(
        program_info,
        scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        m_system.logger(),
        m_system.sender().botbase(),
        m_system.video(),
        m_system.overlay(),
        m_system.audio()
    );

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        run_program_instance(env, scope);
//        m_setup->wait_for_all_requests();
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        e.send_notification(env, m_option.instance().NOTIFICATION_PROGRAM_FINISH);
    }catch (InvalidConnectionStateException&){
    }catch (OperationFailedException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
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
