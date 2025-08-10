/*  Video Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/Redispatch.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
#include "Backends/VideoFrameQt.h"
#include "VideoSources/VideoSource_Null.h"
#include "VideoSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void VideoSession::add_state_listener(StateListener& listener){
    m_state_listeners.add(listener);
}
void VideoSession::remove_state_listener(StateListener& listener){
    m_state_listeners.remove(listener);
}
void VideoSession::add_frame_listener(VideoFrameListener& listener){
    m_frame_listeners.add(listener);
}
void VideoSession::remove_frame_listener(VideoFrameListener& listener){
    m_frame_listeners.remove(listener);
}




VideoSession::~VideoSession(){
    if (m_video_source){
        m_video_source->remove_source_frame_listener(*this);
        m_video_source->remove_rendered_frame_listener(*this);
    }
    global_watchdog().remove(*this);
}
VideoSession::VideoSession(Logger& logger, VideoSourceOption& option)
    : m_logger(logger)
    , m_option(option)
    , m_descriptor(std::make_unique<VideoSourceDescriptor_Null>())
{
    uint8_t watchdog_timeout = GlobalSettings::instance().VIDEO_PIPELINE->AUTO_RESET_SECONDS;
    if (watchdog_timeout != 0){
        global_watchdog().add(*this, std::chrono::seconds(watchdog_timeout));
    }
    set_source(option.descriptor());
}




std::shared_ptr<const VideoSourceDescriptor> VideoSession::descriptor() const{
    ReadSpinLock lg(m_state_lock);
    return m_descriptor;
}
Resolution VideoSession::current_resolution(){
    ReadSpinLock lg(m_state_lock);
    if (m_video_source){
        return m_video_source->current_resolution();
    }else{
        return Resolution();
    }
}
std::vector<Resolution> VideoSession::supported_resolutions() const{
    ReadSpinLock lg(m_state_lock);
    if (m_video_source){
        return m_video_source->supported_resolutions();
    }else{
        return {};
    }
}



void VideoSession::get(VideoSourceOption& option){
    ReadSpinLock lg(m_state_lock);
    option = m_option;
}
void VideoSession::set(const VideoSourceOption& option){
    set_source(option.descriptor(), option.m_resolution);
}

void VideoSession::reset(){
    {
        WriteSpinLock lg(m_queue_lock);
//        cout << "VideoSession::reset(): " << m_queued_commands.size() << endl;
        m_queued_commands.emplace_back(
            Command{
                CommandType::RESET,
                nullptr,
                Resolution{}
            }
        );
    }
    dispatch_to_main_thread([this]{ run_commands(); });
}
void VideoSession::set_source(
    const std::shared_ptr<VideoSourceDescriptor>& device,
    Resolution resolution
){
    {
        WriteSpinLock lg(m_queue_lock);
//        cout << "VideoSession::set_source(): " << m_queued_commands.size() << endl;
        m_queued_commands.emplace_back(
            Command{
                CommandType::SET_SOURCE,
                device,
                resolution
            }
        );
    }
    dispatch_to_main_thread([this]{ run_commands(); });
}
void VideoSession::set_resolution(Resolution resolution){
    {
        WriteSpinLock lg(m_queue_lock);
//        cout << "VideoSession::set_resolution(): " << m_queued_commands.size() << endl;
        m_queued_commands.emplace_back(
            Command{
                CommandType::SET_RESOLUTION,
                nullptr,
                resolution
            }
        );
    }
    dispatch_to_main_thread([this]{ run_commands(); });
}

void VideoSession::internal_reset(){
    m_logger.log("Resetting the video...", COLOR_GREEN);
    m_state_listeners.run_method_unique(&StateListener::pre_shutdown);

    Resolution resolution = m_option.m_resolution;
    std::unique_ptr<VideoSource> source;
    {
        WriteSpinLock lg(m_state_lock);
        source = std::move(m_video_source);
    }
    if (source){
        source->remove_source_frame_listener(*this);
        source->remove_rendered_frame_listener(*this);
        source.reset();
    }

    source = m_descriptor->make_VideoSource(m_logger, resolution);
    if (source){
        resolution = source->current_resolution();
        source->add_source_frame_listener(*this);
        source->add_rendered_frame_listener(*this);
    }

    {
        WriteSpinLock lg(m_state_lock);
        m_option.m_resolution = resolution;
        m_video_source = std::move(source);
    }

    m_state_listeners.run_method_unique(
        &StateListener::post_startup,
        m_video_source.get()
    );
}
void VideoSession::internal_set_source(
    const std::shared_ptr<VideoSourceDescriptor>& device,
    Resolution resolution
){
    m_logger.log("Changing video...", COLOR_GREEN);
    if (*m_descriptor == *device && !m_descriptor->should_reload()){
        return;
    }

    m_state_listeners.run_method_unique(&StateListener::pre_shutdown);

    std::unique_ptr<VideoSource> source;
    {
        WriteSpinLock lg(m_state_lock);
        source = std::move(m_video_source);
    }
    if (source){
        source->remove_source_frame_listener(*this);
        source->remove_rendered_frame_listener(*this);
        source.reset();
    }
    {
        WriteSpinLock lg(m_state_lock);
        m_option.set_descriptor(device);
        m_descriptor = device;
    }

    Resolution desired_resolution = resolution ? resolution : m_option.m_resolution;
    source = device->make_VideoSource(m_logger, desired_resolution);
    if (source){
        resolution = source->current_resolution();
        source->add_source_frame_listener(*this);
        source->add_rendered_frame_listener(*this);
    }

    {
        WriteSpinLock lg(m_state_lock);
        m_option.m_resolution = resolution;
        m_video_source = std::move(source);
    }

    m_state_listeners.run_method_unique(
        &StateListener::post_startup,
        m_video_source.get()
    );
}
void VideoSession::internal_set_resolution(Resolution resolution){
    m_logger.log("Changing resolution...", COLOR_GREEN);
    if (m_option.m_resolution == resolution){
        return;
    }

    m_state_listeners.run_method_unique(&StateListener::pre_shutdown);

    std::unique_ptr<VideoSource> source;
    {
        WriteSpinLock lg(m_state_lock);
        source = std::move(m_video_source);
    }
    if (source){
        source->remove_source_frame_listener(*this);
        source->remove_rendered_frame_listener(*this);
        source.reset();
    }

    source = m_descriptor->make_VideoSource(m_logger, resolution);
    if (source){
        resolution = source->current_resolution();
        source->add_source_frame_listener(*this);
        source->add_rendered_frame_listener(*this);
    }

    {
        WriteSpinLock lg(m_state_lock);
        m_option.m_resolution = resolution;
        m_video_source = std::move(source);
    }

    m_state_listeners.run_method_unique(
        &StateListener::post_startup,
        m_video_source.get()
    );
}

void VideoSession::run_commands(){
    std::lock_guard<std::recursive_mutex> lg0(m_reset_lock);
    if (m_recursion_depth != 0){
        m_logger.log("Suppressing re-entrant command...", COLOR_RED);
        return;
    }
    m_recursion_depth++;
    try{
        while (true){
            Command command;
            {
                WriteSpinLock lg(m_queue_lock);
//                cout << "VideoSession::run_commands(): " << m_queued_commands.size() << endl;
                if (m_queued_commands.empty()){
                    break;
                }
                command = std::move(m_queued_commands.front());
                m_queued_commands.pop_front();
            }

            switch (command.command_type){
            case CommandType::RESET:
                internal_reset();
                break;

            case CommandType::SET_SOURCE:
                internal_set_source(command.device, command.resolution);
                break;

            case CommandType::SET_RESOLUTION:
                internal_set_resolution(command.resolution);
                break;
            }
        }
        m_recursion_depth--;
    }catch (...){
        m_recursion_depth--;
        throw;
    }
}


VideoSnapshot VideoSession::snapshot_latest_blocking(){
    ReadSpinLock lg(m_state_lock);
    if (m_video_source){
        return m_video_source->snapshot_latest_blocking();
    }else{
        return VideoSnapshot();
    }
}
VideoSnapshot VideoSession::snapshot_recent_nonblocking(WallClock min_time){
    ReadSpinLock lg(m_state_lock);
    if (m_video_source){
        return m_video_source->snapshot_recent_nonblocking(min_time);
    }else{
        return VideoSnapshot();
    }
}

double VideoSession::fps_source() const{
    ReadSpinLock lg(m_fps_lock);
    return m_fps_tracker_source.events_per_second();
}
double VideoSession::fps_display() const{
    ReadSpinLock lg(m_fps_lock);
    return m_fps_tracker_rendered.events_per_second();
}
void VideoSession::on_frame(std::shared_ptr<const VideoFrame> frame){
    m_frame_listeners.run_method_unique(&VideoFrameListener::on_frame, frame);
    {
        WriteSpinLock lg(m_fps_lock);
        m_fps_tracker_source.push_event(frame->timestamp);
    }
    global_watchdog().delay(*this);
}
void VideoSession::on_rendered_frame(WallClock timestamp){
    WriteSpinLock lg(m_fps_lock);
    m_fps_tracker_rendered.push_event(timestamp);
}


void VideoSession::on_watchdog_timeout(){
    {
        ReadSpinLock lg(m_state_lock);
        if (!m_video_source || !m_video_source->allow_watchdog_reset()){
            return;
        }
    }

    uint8_t watchdog_timeout = GlobalSettings::instance().VIDEO_PIPELINE->AUTO_RESET_SECONDS;
    m_logger.log("No video detected for " + std::to_string(watchdog_timeout) + " seconds...", COLOR_RED);

    if (watchdog_timeout == 0){
        return;
    }

    reset();
}

















}
