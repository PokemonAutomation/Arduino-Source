/*  Video Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "VideoSource.h"

namespace PokemonAutomation{



VideoSource::VideoSource(Logger& m_logger, bool allow_watchdog_reset)
    : m_logger(m_logger)
    , m_allow_watchdog_reset(allow_watchdog_reset)
    , m_stats_report_source_frame("VideoSource::report_source_frame()", "ms", 1000, std::chrono::seconds(60))
    , m_stats_report_rendered_frame("VideoSource::report_rendered_frame()", "ms", 1000, std::chrono::seconds(60))
    , m_sanitizer("VideoSource")
{}


void VideoSource::report_source_frame(std::shared_ptr<const VideoFrame> frame){
    auto scope_check = m_sanitizer.check_scope();
    WallClock time0 = current_time();
    m_source_frame_listeners.run_method_unique(&VideoFrameListener::on_frame, frame);
    WallClock time1 = current_time();
    auto microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
    m_stats_report_source_frame.report_data(m_logger, microseconds);
}
void VideoSource::report_rendered_frame(WallClock timestamp){
    auto scope_check = m_sanitizer.check_scope();
    WallClock time0 = current_time();
    m_rendered_frame_listeners.run_method_unique(&RenderedFrameListener::on_rendered_frame, timestamp);
    WallClock time1 = current_time();
    auto microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
    m_stats_report_rendered_frame.report_data(m_logger, microseconds);
}



}
