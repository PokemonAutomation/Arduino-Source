/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_MultiSwitchProgramOption.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void MultiSwitchProgramSession::add_listener(Listener& listener){
    WriteSpinLock lg(m_lock);
    m_listeners.insert(&listener);
}
void MultiSwitchProgramSession::remove_listener(Listener& listener){
    WriteSpinLock lg(m_lock);
    m_listeners.erase(&listener);
}




MultiSwitchProgramSession::MultiSwitchProgramSession(MultiSwitchProgramOption& option)
    : ProgramSession(option.descriptor())
    , m_option(option)
    , m_system(option.system(), instance_id())
    , m_scope(nullptr)
{
    WriteSpinLock lg(m_lock);
    m_system.add_listener(*this);
    m_option.instance().update_active_consoles(option.system().count());
}

MultiSwitchProgramSession::~MultiSwitchProgramSession(){
    MultiSwitchProgramSession::internal_stop_program();
    m_system.remove_listener(*this);
    join_program_thread();
}



void MultiSwitchProgramSession::restore_defaults(){
    std::lock_guard<std::mutex> lg(program_lock());
    if (current_state() != ProgramState::STOPPED){
        logger().log("Cannot change settings while program is running.", COLOR_RED);
        return;
    }
    logger().log("Restoring settings to defaults...");
    m_option.restore_defaults();
}
std::string MultiSwitchProgramSession::check_validity() const{
    return m_option.check_validity();
}



void MultiSwitchProgramSession::run_program_instance(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    {
        std::lock_guard<std::mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    size_t consoles = m_system.count();
    for (size_t c = 0; c < consoles; c++){
        if (!m_system[c].serial_session().is_ready()){
            throw UserSetupError(m_system[c].logger(), "Cannot Start: Serial connection not ready.");
        }
        start_program_video_check(env.consoles[c], m_option.descriptor().feedback());
    }

    m_scope.store(&scope, std::memory_order_release);

    try{
        m_option.instance().program(env, scope);
        for (size_t c = 0; c < consoles; c++){
            env.consoles[c].botbase().wait_for_all_requests();
        }
    }catch (...){
        m_scope.store(nullptr, std::memory_order_release);
        throw;
    }
    m_scope.store(nullptr, std::memory_order_release);
}
void MultiSwitchProgramSession::internal_stop_program(){
    WriteSpinLock lg(m_lock);
    size_t consoles = m_system.count();
    for (size_t c = 0; c < consoles; c++){
        m_system[c].serial_session().stop();
    }
    CancellableScope* scope = m_scope.load(std::memory_order_acquire);
    if (scope != nullptr){
        scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
    }

    //  Wait for program thread to finish.
    while (m_scope.load(std::memory_order_acquire) != nullptr){
        pause();
    }

    for (size_t c = 0; c < consoles; c++){
        m_system[c].serial_session().reset();
    }
}
void MultiSwitchProgramSession::internal_run_program(){
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread();
    m_option.options().reset_state();

    SleepSuppressScope sleep_scope(GlobalSettings::instance().SLEEP_SUPPRESS.PROGRAM_RUNNING);

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
        handles.emplace_back(
            c,
            session.logger(),
            session.sender().botbase(),
            session.video(),
            session.overlay(),
            session.audio()
        );
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


void MultiSwitchProgramSession::shutdown(){
    internal_stop_program();
}
void MultiSwitchProgramSession::startup(size_t switch_count){
    WriteSpinLock lg(m_lock);
    m_option.instance().update_active_consoles(switch_count);
    for (Listener* listener : m_listeners){
        listener->redraw_options();
    }
}






}
}
