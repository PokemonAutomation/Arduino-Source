/*  Console System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/VideoPipeline/Stats/MemoryUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "Integrations/ProgramTracker.h"
#include "Controllers/NullController.h"
#include "ConsoleSystemSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ConsoleInfra{


bool ConsoleSystemSession::try_shutdown() noexcept{
    bool success = true;

    m_video.remove_state_listener(m_history);
    m_video.remove_state_listener(m_history);
    m_audio.remove_stream_listener(m_history);
    m_audio.remove_state_listener(m_history);

    m_overlay.remove_stat(*m_main_thread_utilization);
    m_overlay.remove_stat(*m_cpu_utilization);
    m_overlay.remove_stat(m_memory_usage->m_process);
    m_overlay.remove_stat(m_memory_usage->m_system);

    success &= m_video.try_shutdown();

    return success;
}
ConsoleSystemSession::~ConsoleSystemSession(){
    ProgramTracker::instance().remove_console(m_console_tracking_id);
    blocking_shutdown(
        m_logger,
        "ConsoleSystemSession",
        [this]{ return ConsoleSystemSession::try_shutdown(); }
    );
}
ConsoleSystemSession::ConsoleSystemSession(
    Logger& logger,
    ConsoleSystemOption& option,
    size_t console_number,
    std::optional<uint64_t> program_tracking_id
)
    : m_console_number(console_number)
    , m_logger(logger, "Console " + std::to_string(console_number))
    , m_option(option)
    , m_video(m_logger, option.m_video)
    , m_audio(m_logger, option.m_audio)
    , m_overlay(m_logger, option.m_overlay)
    , m_controllers(option.m_controllers.size())
    , m_history(m_logger)
    , m_memory_usage(new MemoryUtilizationStats())
    , m_cpu_utilization(new CpuUtilizationStat())
    , m_main_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Main Qt Thread:"))
{
    if (option.m_controllers.size() == 1){
        m_controllers.emplace_back(m_logger, option.m_controllers[0]);
    }else{
        for (ControllerOption& controller : option.m_controllers){
            m_controllers.emplace_back(m_logger, m_controllers.size(), controller);
        }
    }

    m_history.start(m_audio.input_format(), m_video.current_source() != nullptr);

    try{
        m_overlay.add_stat(m_memory_usage->m_system);
        m_overlay.add_stat(m_memory_usage->m_process);
        m_overlay.add_stat(*m_cpu_utilization);
        m_overlay.add_stat(*m_main_thread_utilization);

        m_audio.add_state_listener(m_history);
        m_audio.add_stream_listener(m_history);
        m_video.add_state_listener(m_history);
        m_video.add_frame_listener(m_history);

        m_overlay.add_hid_listener(*this);
    }catch (...){
        ConsoleSystemSession::try_shutdown();
        throw;
    }

    m_console_tracking_id = ProgramTracker::instance().add_console(program_tracking_id, *this);
}


void ConsoleSystemSession::save(ConsoleSystemOption& option) const{
    std::lock_guard<Mutex> lg(m_lock);

    m_video.save(option.m_video);
    m_audio.save(option.m_audio);
    m_overlay.save(option.m_overlay);

    option.m_controllers = m_option.m_controllers;
}
void ConsoleSystemSession::load(const ConsoleSystemOption& option){
    std::lock_guard<Mutex> lg(m_lock);

    m_video.load(option.m_video);
    m_audio.load(option.m_audio);
    m_overlay.load(option.m_overlay);

    size_t c = 0;

    size_t stop = std::min(m_option.m_controllers.size(), option.m_controllers.size());
    for (; c < stop; c++){
        m_option.m_controllers[c] = option.m_controllers[c];
        m_controllers[c].session.load(option.m_controllers[c]);
    }

    stop = m_option.m_controllers.size();
    for (; c < stop; c++){
        m_option.m_controllers[c].set_descriptor(std::make_shared<NullControllerDescriptor>());
        m_controllers[c].session.set_device(m_option.m_controllers[c].descriptor());
    }
}


void ConsoleSystemSession::lock_controllers(std::string reason){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_lock_controllers_reason = std::move(reason);
        for (ControllerEntry& controller : m_controllers){
            controller.session.set_options_locked(true);
        }
    }
    m_listeners.run_method(&Listener::on_lock_controllers);
}
void ConsoleSystemSession::unlock_controllers(){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_lock_controllers_reason.clear();
        global_input_clear_state();
        for (ControllerEntry& controller : m_controllers){
            controller.session.set_options_locked(false);
        }
    }
    m_listeners.run_method(&Listener::on_unlock_controllers);
}
void ConsoleSystemSession::save_history(const std::string& filename){
    m_history.save(filename);
}


void ConsoleSystemSession::on_focus_in(){
//    cout << "ConsoleSystemSession::on_focus_in()" << endl;
    std::lock_guard<Mutex> lg(m_lock);
    if (m_focused){
        return;
    }
    m_focused = true;

    global_input_add_listener(*this);
}
void ConsoleSystemSession::on_focus_out(){
//    cout << "ConsoleSystemSession::on_focus_out()" << endl;
    std::lock_guard<Mutex> lg(m_lock);
    if (!m_focused){
        return;
    }
    m_focused = false;
    if (!m_lock_controllers_reason.empty() && !allow_commands_while_running()){
        return;
    }

    global_input_clear_state();
    global_input_remove_listener(*this);

    for (ControllerEntry& controller : m_controllers){
        std::string error = controller.session.try_run<AbstractController>([](AbstractController& controller){
            controller.cancel_all_commands();
        });
        if (!error.empty()){
            controller.session.logger().log(error, COLOR_RED);
        }
    }
}
void ConsoleSystemSession::run_controller_input(ControllerInputState& state){
    std::lock_guard<Mutex> lg(m_lock);
    if (!m_focused){
        m_logger.log("Keyboard Command Suppressed: Not in focus.", COLOR_RED);
        return;
    }
    if (!m_lock_controllers_reason.empty() && !allow_commands_while_running()){
        m_logger.log("Keyboard Command Suppressed: " + m_lock_controllers_reason, COLOR_RED);
        return;
    }

//    cout << "ConsoleSystemSession::run_controller_input()" << endl;
    for (ControllerEntry& controller : m_controllers){
        std::string error = controller.session.try_run<AbstractController>([&](AbstractController& controller){
            controller.run_controller_input(state);
        });
        if (!error.empty()){
            controller.session.logger().log("Keyboard Command Failed: " + error, COLOR_RED);
        }
    }
}






}
}
