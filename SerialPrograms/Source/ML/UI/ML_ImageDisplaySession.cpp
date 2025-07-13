/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "Integrations/ProgramTracker.h"
#include "ML_ImageDisplayOption.h"
#include "ML_ImageDisplaySession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{



ImageDisplaySession::~ImageDisplaySession(){
    try{
        m_logger.log("Shutting down ImageDisplaySession...");
    }catch (...){}

    m_overlay.remove_stat(*m_main_thread_utilization);
    m_overlay.remove_stat(*m_cpu_utilization);
}
ImageDisplaySession::ImageDisplaySession(
    ImageDisplayOption& option,
    uint64_t program_id,
    size_t console_number
)
    : m_logger(global_logger_raw(), "Console " + std::to_string(console_number))
    , m_video(m_logger, option.m_video)
    , m_overlay(option.m_overlay)
    , m_cpu_utilization(new CpuUtilizationStat())
    , m_main_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Main Qt Thread:"))
{
    m_overlay.add_stat(*m_cpu_utilization);
    m_overlay.add_stat(*m_main_thread_utilization);
}


void ImageDisplaySession::get(ImageDisplayOption& option){
    m_video.get(option.m_video);
    m_overlay.get(option.m_overlay);
}
void ImageDisplaySession::set(const ImageDisplayOption& option){
    m_video.set(option.m_video);
    m_overlay.set(option.m_overlay);
}





}
}
