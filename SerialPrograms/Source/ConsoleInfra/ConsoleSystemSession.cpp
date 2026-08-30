/*  Console System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "CommonFramework/VideoPipeline/Stats/MemoryUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "ConsoleSystemSession.h"

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
    blocking_shutdown(
        m_logger,
        "ConsoleSystemSession",
        [this]{ return ConsoleSystemSession::try_shutdown(); }
    );
}
ConsoleSystemSession::ConsoleSystemSession(
    Logger& logger,
    ConsoleSystemOption& option,
    size_t console_number
)
    : m_console_number(console_number)
    , m_logger(logger, "Console " + std::to_string(console_number))
    , m_option(option)
    , m_video(m_logger, option.m_video)
    , m_audio(m_logger, option.m_audio)
    , m_overlay(m_logger, option.m_overlay)
    , m_history(m_logger)
    , m_memory_usage(new MemoryUtilizationStats())
    , m_cpu_utilization(new CpuUtilizationStat())
    , m_main_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Main Qt Thread:"))
{
    for (ControllerOption& controller : option.m_controllers){
        m_controllers.emplace_back(std::make_unique<ControllerSession>(m_logger, controller));
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
    }catch (...){
        ConsoleSystemSession::try_shutdown();
        throw;
    }
}


void ConsoleSystemSession::get(ConsoleSystemOption& option) const{
    m_video.get(option.m_video);
    m_audio.get(option.m_audio);
    m_overlay.get(option.m_overlay);

    option.m_controllers.resize(m_controllers.size());
    for (size_t c = 0; c < m_controllers.size(); c++){
        m_controllers[c]->get(option.m_controllers[c]);
    }
}
void ConsoleSystemSession::set(const ConsoleSystemOption& option){
    m_video.set(option.m_video);
    m_audio.set(option.m_audio);
    m_overlay.set(option.m_overlay);

    m_option.m_controllers.resize(option.m_controllers.size());
    m_controllers.resize(option.m_controllers.size());
    for (size_t c = 0; c < m_controllers.size(); c++){
        std::unique_ptr<ControllerSession>& controller = m_controllers[c];
        if (controller == nullptr){
            controller = std::make_unique<ControllerSession>(m_logger, m_option.m_controllers[c]);
        }else{
            m_controllers[c]->set(option.m_controllers[c]);
        }
    }
}

void ConsoleSystemSession::set_allow_user_commands(const std::string& disallow_reason){
    for (std::unique_ptr<ControllerSession>& controller : m_controllers){
        controller->set_user_input_blocked(disallow_reason);
    }
}
void ConsoleSystemSession::save_history(const std::string& filename){
    m_history.save(filename);
}






}
}
