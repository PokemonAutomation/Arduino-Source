/*  Computer Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ComputerProgramSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ComputerProgramSession::ComputerProgramSession(const ComputerProgramDescriptor& descriptor)
    : UiState<ComputerProgramSession, PanelSession>(descriptor)
    , ProgramSession(descriptor)
    , m_descriptor(descriptor)
    , m_instance(descriptor.make_instance())
{}

ComputerProgramSession::~ComputerProgramSession(){
    ComputerProgramSession::internal_stop_program();
    join_program_thread();
}


ConfigOption& ComputerProgramSession::options(){
    return m_instance->m_options;
}
std::string ComputerProgramSession::check_validity() const{
    return m_instance->check_validity();
}




void ComputerProgramSession::restore_defaults(){
    std::lock_guard<Mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_instance->restore_defaults();
}
JsonValue ComputerProgramSession::to_json() const{
    return m_instance->to_json();
}
void ComputerProgramSession::load_json(const JsonValue& json){
    m_instance->load_json(json);
}


void ComputerProgramSession::run_program_instance(ProgramEnvironment& env, CancellableScope& scope){
    {
        std::lock_guard<Mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }

    m_instance->program(env, scope);
}
void ComputerProgramSession::internal_stop_program(){
    {
        std::lock_guard<Mutex> lg(program_lock());
        if (m_scope != nullptr){
            m_scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
        }
    }
    wait_for_finish();
}
void ComputerProgramSession::internal_run_program(){
    RunningProgramScope running_scope(*this);

    {
        std::lock_guard<Mutex> lg(program_lock());
        if (current_state() != ProgramState::RUNNING){
            return;
        }
    }
    if (!download_prereqs(*m_scope)){
        return;
    }

    options().reset_state();

    ProgramInfo program_info(
        identifier(),
        m_descriptor.category(),
        m_descriptor.display_name(),
        last_state_change()
    );
    ProgramEnvironment env(
        program_info,
        *this,
        current_stats_tracker(), historical_stats_tracker()
    );

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        run_program_instance(env, *m_scope);
//        m_setup->wait_for_all_requests();
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException& e){
        logger().log("Program finished early!", COLOR_BLUE);
        send_program_finished_notification(env, m_instance->NOTIFICATION_PROGRAM_FINISH, e.message(), *e.screenshot());
    }catch (InvalidConnectionStateException&){
    }catch (OperationFailedExceptionWithScreenshot& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (OperationFailedException& e){ // no screenshot
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (FatalProgramException& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        e.send_fatal_error_notif_and_telemetry_report(env, m_instance->NOTIFICATION_ERROR_FATAL);
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
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
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            message
        );
    }catch (...){
        logger().log("Program stopped with an exception!", COLOR_RED);
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            env, m_instance->NOTIFICATION_ERROR_FATAL,
            "Unknown error."
        );
    }
}



}
