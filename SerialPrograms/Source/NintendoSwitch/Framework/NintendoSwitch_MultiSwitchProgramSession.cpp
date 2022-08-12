/*  Multi-Switch Program Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/FixedLimitVector.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/BlackBorderCheck.h"
#include "NintendoSwitch_MultiSwitchProgramOption.h"
#include "NintendoSwitch_MultiSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void MultiSwitchProgramSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void MultiSwitchProgramSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}




MultiSwitchProgramSession::MultiSwitchProgramSession(MultiSwitchProgramOption& option)
    : ProgramSession(option.descriptor())
    , m_option(option)
    , m_system(option.system(), instance_id())
{
    std::unique_lock<std::mutex> lg(m_lock);
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



void MultiSwitchProgramSession::run_program_instance(const ProgramInfo& info){
    {
        std::lock_guard<std::mutex> lg(program_lock());
        std::string error = check_validity();
        if (!error.empty()){
            throw UserSetupError(logger(), std::move(error));
        }
    }

    //  Acquire lock here to block the session from changing the # of consoles.
    std::unique_lock<std::mutex> lg(m_lock);

    size_t consoles = m_system.count();
    for (size_t c = 0; c < consoles; c++){
        if (!m_system[c].serial_session().is_ready()){
            throw UserSetupError(m_system[c].logger(), "Cannot Start: Serial connection not ready.");
        }
    }

    FixedLimitVector<ConsoleHandle> handles(consoles);
    for (size_t c = 0; c < consoles; c++){
        SwitchSystemSession& session = m_system[c];
        handles.emplace_back(
            c,
            session.logger(),
            *session.sender().botbase(),
            session.video(),
            session.overlay(),
            session.audio()
        );
        start_program_video_check(handles.back(), m_option.descriptor().feedback());
    }


    CancellableHolder<CancellableScope> scope;
    std::unique_ptr<MultiSwitchProgramEnvironment> env(new MultiSwitchProgramEnvironment(
        info,
        scope,
        *this,
        current_stats_tracker(), historical_stats_tracker(),
        std::move(handles)
    ));

    m_scope = &scope;

    //  Now we can safely unlock.
    //  If the session changes the # of switches here, it will safely fall
    //  through the regular cancellation path.
    lg.unlock();


    try{
        m_option.instance().program(*env, scope);
    }catch (...){
        lg.lock();
        m_scope = nullptr;
        m_cv.notify_all();
        throw;
    }
    lg.lock();
    m_scope = nullptr;
    m_cv.notify_all();
}
void MultiSwitchProgramSession::internal_stop_program(){
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_scope == nullptr){
        return;
    }
    m_scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
    m_cv.wait(lg, [=]{ return m_scope == nullptr; });
}
void MultiSwitchProgramSession::internal_run_program(){
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread();

    ProgramInfo program_info(
        identifier(),
        m_option.descriptor().category(),
        m_option.descriptor().display_name(),
        timestamp()
    );

    try{
        logger().log("<b>Starting Program: " + identifier() + "</b>");
        run_program_instance(program_info);
//        m_setup->wait_for_all_requests();
        logger().log("Program finished normally!", COLOR_BLUE);
    }catch (OperationCancelledException&){
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException&){
        logger().log("Program finished early!", COLOR_BLUE);
        send_program_finished_notification(
            logger(), m_option.instance().NOTIFICATION_PROGRAM_FINISH,
            program_info,
            "",
            current_stats(),
            historical_stats()
        );
    }catch (InvalidConnectionStateException&){
    }catch (Exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.message();
        if (message.empty()){
            message = e.name();
        }
        report_error(message);
        send_program_fatal_error_notification(
            logger(), m_option.instance().NOTIFICATION_ERROR_FATAL,
            program_info,
            std::move(message),
            current_stats(),
            historical_stats()
        );
    }catch (std::exception& e){
        logger().log("Program stopped with an exception!", COLOR_RED);
        std::string message = e.what();
        if (message.empty()){
            message = "Unknown std::exception.";
        }
        report_error(message);
        send_program_fatal_error_notification(
            logger(), m_option.instance().NOTIFICATION_ERROR_FATAL,
            program_info,
            std::move(message),
            current_stats(),
            historical_stats()
        );
    }catch (...){
        logger().log("Program stopped with an exception!", COLOR_RED);
        report_error("Unknown error.");
        send_program_fatal_error_notification(
            logger(), m_option.instance().NOTIFICATION_ERROR_FATAL,
            program_info,
            "Unknown error.",
            current_stats(),
            historical_stats()
        );
    }
}


void MultiSwitchProgramSession::shutdown(){
    internal_stop_program();
}
void MultiSwitchProgramSession::startup(size_t switch_count){
    std::unique_lock<std::mutex> lg(m_lock);
    m_option.instance().update_active_consoles(switch_count);
    for (Listener* listener : m_listeners){
        listener->redraw_options();
    }
}






}
}
