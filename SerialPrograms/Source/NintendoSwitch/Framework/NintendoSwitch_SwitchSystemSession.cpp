/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/Stats/MemoryUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "Integrations/ProgramTracker.h"
#include "NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch_SwitchSystemSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemSession::~SwitchSystemSession(){
    try{
        m_logger.log("Shutting down SwitchSystemSession...");
    }catch (...){}

    m_video.remove_state_listener(m_history);
    m_video.add_frame_listener(m_history);
    m_audio.remove_stream_listener(m_history);
    m_audio.remove_state_listener(m_history);

    ProgramTracker::instance().remove_console(m_console_id);
    m_overlay.remove_stat(*m_main_thread_utilization);
    m_overlay.remove_stat(*m_cpu_utilization);
    m_overlay.remove_stat(m_memory_usage->m_process);
    m_overlay.remove_stat(m_memory_usage->m_system);
}
SwitchSystemSession::SwitchSystemSession(
    SwitchSystemOption& option,
    uint64_t program_id,
    size_t console_number
)
    : m_console_number(console_number)
    , m_logger(global_logger_raw(), "Console " + std::to_string(console_number))
    , m_option(option)
    , m_controller(m_logger, option.m_controller)
    , m_video(m_logger, option.m_video)
    , m_audio(m_logger, option.m_audio)
    , m_overlay(m_logger, option.m_overlay)
    , m_history(m_logger)
    , m_memory_usage(new MemoryUtilizationStats())
    , m_cpu_utilization(new CpuUtilizationStat())
    , m_main_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Main Qt Thread:"))
{
    m_console_id = ProgramTracker::instance().add_console(program_id, *this);
    m_overlay.add_stat(m_memory_usage->m_system);
    m_overlay.add_stat(m_memory_usage->m_process);
    m_overlay.add_stat(*m_cpu_utilization);
    m_overlay.add_stat(*m_main_thread_utilization);

    m_history.start(m_audio.input_format(), m_video.current_source() != nullptr);

    m_audio.add_state_listener(m_history);
    m_audio.add_stream_listener(m_history);
    m_video.add_state_listener(m_history);
    m_video.add_frame_listener(m_history);
}


void SwitchSystemSession::get(SwitchSystemOption& option){
    m_controller.get(option.m_controller);
    m_video.get(option.m_video);
    m_audio.get(option.m_audio);
    m_overlay.get(option.m_overlay);
}
void SwitchSystemSession::set(const SwitchSystemOption& option){
    m_controller.set(option.m_controller);
    m_video.set(option.m_video);
    m_audio.set(option.m_audio);
    m_overlay.set(option.m_overlay);
}

void SwitchSystemSession::set_allow_user_commands(std::string disallow_reason){
    m_controller.set_user_input_blocked(std::move(disallow_reason));
}
void SwitchSystemSession::save_history(const std::string& filename){
    m_history.save(filename);
}






}
}
