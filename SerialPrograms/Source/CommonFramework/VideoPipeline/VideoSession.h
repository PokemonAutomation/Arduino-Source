/*  Video Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSession_H
#define PokemonAutomation_VideoPipeline_VideoSession_H

#include <memory>
#include <mutex>
#include "Common/Cpp/EventRateTracker.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "VideoSourceDescriptor.h"
#include "VideoSource.h"

namespace PokemonAutomation{


class VideoSession
    : public VideoFeed
    , private VideoFrameListener
    , private VideoSource::RenderedFrameListener
    , private WatchdogCallback
{
public:
    struct StateListener{
        virtual void post_startup(VideoSource* source){}

        //  Sent before the video source shuts down. Listeners should drop their
        //  references to the internal video source before returning.
        virtual void pre_shutdown(){}
        virtual void post_shutdown(){}

        virtual void pre_resolution_change(Resolution resolution){}
        virtual void post_resolution_change(Resolution resolution){}
    };

    void add_state_listener(StateListener& listener);
    void remove_state_listener(StateListener& listener);

    virtual void add_frame_listener(VideoFrameListener& listener) override;
    virtual void remove_frame_listener(VideoFrameListener& listener) override;


public:
    ~VideoSession();
    VideoSession(Logger& logger, VideoSourceOption& option);

    Logger& logger(){
        return m_logger;
    }

    std::shared_ptr<const VideoSourceDescriptor> descriptor() const;

    //  Warning, not thread-safe with any source changes.
    VideoSource* current_source(){
        return m_video_source.get();
    }

    Resolution current_resolution();
    std::vector<Resolution> supported_resolutions() const;

    virtual VideoSnapshot snapshot_latest_blocking() override;
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override;

    virtual double fps_source() const override;
    virtual double fps_display() const override;


public:
    void get(VideoSourceOption& option);
    void set(const VideoSourceOption& option);

    virtual void reset() override;
    void set_source(const std::shared_ptr<VideoSourceDescriptor>& device);
    void set_resolution(Resolution resolution);


private:
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;
    virtual void on_rendered_frame(WallClock timestamp) override;

    virtual void on_watchdog_timeout() override;


private:
    mutable std::mutex m_reset_lock;
    mutable SpinLock m_state_lock;

    Logger& m_logger;
    VideoSourceOption& m_option;

    mutable SpinLock m_fps_lock;
    EventRateTracker m_fps_tracker_source;
    EventRateTracker m_fps_tracker_rendered;

    std::shared_ptr<const VideoSourceDescriptor> m_descriptor;
    std::unique_ptr<VideoSource> m_video_source;

    ListenerSet<StateListener> m_state_listeners;
    ListenerSet<VideoFrameListener> m_frame_listeners;
};




}
#endif
