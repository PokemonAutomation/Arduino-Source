/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "Integrations/ProgramTracker.h"
#include "ML_ImageAnnotationDisplayOption.h"
#include "ML_ImageAnnotationDisplaySession.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{

VideoSourceOption create_still_image_video_source_option(){
    VideoSourceOption option;
    option.set_descriptor(option.get_descriptor_from_cache(VideoSourceType::StillImage));
    return option;
}

ImageAnnotationDisplaySession::~ImageAnnotationDisplaySession(){
    try{
        m_logger.log("Shutting down ImageAnnotationDisplaySession...");
    }catch (...){}

    m_overlay.remove_stat(*m_main_thread_utilization);
    m_overlay.remove_stat(*m_cpu_utilization);
}
ImageAnnotationDisplaySession::ImageAnnotationDisplaySession(ImageAnnotationDisplayOption& option)
    : m_logger(global_logger_raw(), "ML")
    , m_display_option(option)
    , m_still_image_option(create_still_image_video_source_option())
    , m_video_session(m_logger, m_still_image_option)
    , m_overlay(m_logger, option.m_overlay)
    , m_cpu_utilization(new CpuUtilizationStat())
    , m_main_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Main Qt Thread:"))
{
    m_overlay.add_stat(*m_cpu_utilization);
    m_overlay.add_stat(*m_main_thread_utilization);

    m_still_image_descriptor = std::make_shared<VideoSourceDescriptor_StillImage>();
}


void ImageAnnotationDisplaySession::set_image_source(const std::string& path){
    m_display_option.m_image_path = path;
    m_still_image_descriptor->set_path(path);
    m_video_session.set_source(m_still_image_descriptor);
}





}
}
