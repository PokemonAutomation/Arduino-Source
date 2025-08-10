/*  Video Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSession_H
#define PokemonAutomation_VideoPipeline_VideoSession_H

#include <memory>
#include <deque>
#include <mutex>
#include "Common/Cpp/EventRateTracker.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "VideoSourceDescriptor.h"
#include "VideoSource.h"

namespace PokemonAutomation{


class VideoSession
    : public VideoFeed  // interface class the automation programs have access to to query video snapshots
                        // and reset the video source
    , private VideoFrameListener  // listens to incoming frames from video source
    , private VideoSource::RenderedFrameListener  // listens to newly rendered frame from video source
    , private WatchdogCallback
{
public:
    //  other classes can inherit this struct to react to state changes in the VideoSession.
    //  You need to call VideoSession::add_state_listener() to add other classes as listeners
    //  and remove them by VideoSession::remove_state_listener() when they are no longer needed.
    //  When a state change happens, VideoSession will call the corresponding callback functions
    //  in the StateListener.
    struct StateListener{
        //  Called whenever the video source is initialized or reset.
        //  This also includes when the video resolution is changed as we need to rebuild the video
        //  source with the new resolution.
        virtual void post_startup(VideoSource* source){}

        //  Sent before the video source resets or shuts down. Listeners should drop their
        //  references to the internal video source before returning.
        //  This also includes when the video resolution is changed as we need to rebuild the video
        //  source with the new resolution.
        virtual void pre_shutdown(){}
        //  Currently not called by VideoSession yet.
        virtual void post_shutdown(){}
        //  Currently not called by VideoSession yet.
        virtual void pre_resolution_change(Resolution resolution){}
        //  Currently not called by VideoSession yet.
        virtual void post_resolution_change(Resolution resolution){}
    };

    //  Add a state listener for it to react to state changes in the VideoSession.
    //  When a state change happens, VideoSession will call the corresponding callback functions
    //  in the StateListener.
    void add_state_listener(StateListener& listener);
    //  Remove the state listener.
    void remove_state_listener(StateListener& listener);

    //  Implements VideoFeed::add_frame_listener().
    //  Add a VideoFrameListener for it to react to incoming frames in the video source.
    //  Whenever the video source sends a frame, VideoSession will call the callback functions
    //  in added listeners.
    virtual void add_frame_listener(VideoFrameListener& listener) override;
    //  Implements VideoFeed::remove_frame_listener().
    //  Remove the frame listener.
    virtual void remove_frame_listener(VideoFrameListener& listener) override;


public:
    ~VideoSession();
    //  Built from a VideoSourceOption.
    //  VideoSourceOption manages the current VideoSourceDescriptor, which are the descriptors
    //  to build a VideoSource.
    //  VideoSession stores a reference of the passed in VideoSourceOption to manipulate the
    //  option according to user selection.
    VideoSession(Logger& logger, VideoSourceOption& option);

    Logger& logger(){
        return m_logger;
    }

    //  Current video source descriptor.
    //  This function is thread-safe. It has a lock to prevent concurrent calls
    //  of other VideoSession functions.
    std::shared_ptr<const VideoSourceDescriptor> descriptor() const;

    //  Warning, not thread-safe with any source changes.
    VideoSource* current_source(){
        return m_video_source.get();
    }

    //  Return current resolution.
    //  This function is thread-safe. It has a lock to prevent concurrent calls
    //  of other VideoSession functions.
    Resolution current_resolution();
    //  Return supported resolutions.
    //  This function is thread-safe. It has a lock to prevent concurrent calls
    //  of other VideoSession functions.
    std::vector<Resolution> supported_resolutions() const;

    //  Implements VideoFeed::snapshot() to provide a snapshot from the current video stream.
    //  It will wait for a latest snapshot, blocking the current thread to wait for it.
    //  This function is thread-safe. It has a lock to prevent concurrent calls
    //  of other VideoSession functions.
    virtual VideoSnapshot snapshot_latest_blocking() override;
    //  Implements VideoFeed::snapshot_recent_nonblocking() to provide a snapshot from the
    //  current video stream.
    //  It will use the most recent available frame, therefore non-blocking.
    //  This function is thread-safe. It has a lock to prevent concurrent calls
    //  of other VideoSession functions.
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override;

    //  Implements VideoFeed::fps_source().
    //  Returns the currently measured frames/second for the video source.
    //  Use this for diagnostic purposes.
    //  This function is thread-safe. It has a lock to prevent concurrent fps calls.
    virtual double fps_source() const override;
    //  Implements VideoFeed::fps_display().
    //  Returns the currently measured frames/second for the video display thread.
    //  Use this for diagnostic purposes.
    //  This function is thread-safe. It has a lock to prevent concurrent fps calls.
    virtual double fps_display() const override;


public:
    //  Get current video source option
    void get(VideoSourceOption& option);
    //  Set a new video source. This will close the old video source.
    //  This equals to calling VideoSession::set_source(option.descriptor()).
    //  Change of video source and resolution will be reflected on the internal
    //  referenced video source option (aka the VideoSourceOption passed to the
    //  VideoSession constructor).
    //  The change is dispatched to the Qt main thread to execute.
    void set(const VideoSourceOption& option);
    //  Implements VideoFeed::reset().
    //  Reset the current video source. This equals to close the old video source
    //  and reopen it.
    //  This may also change the internal referenced video source option (aka the
    //  VideoSourceOption passed to the VideoSession constructor).
    //  The change is dispatched to the Qt main thread to execute.
    virtual void reset() override;
    //  Set a new video source. This will close the old video source.
    //  Change of video source and resolution will be reflected on the internal
    //  referenced video source option (aka the VideoSourceOption passed to the
    //  VideoSession constructor).
    //  The change is dispatched to the Qt main thread to execute.
    void set_source(
        const std::shared_ptr<VideoSourceDescriptor>& device,
        Resolution resolution = {}
    );
    //  Change video resolution.
    //  This will trigger a video source reset.
    //  Change of resolution will be reflected on the internal referenced video
    //  source option (aka the VideoSourceOption passed to the VideoSession
    //  constructor).
    //  The change is dispatched to the Qt main thread to execute.
    void set_resolution(Resolution resolution);


private:
    //  Overwrites VideoFrameListener::on_frame()
    //  When this function is called, the VideoSession will update its internal fps record
    //  and call frame listeners added by VideoSession::add_frame_listener().
    //  This function is thread-safe as it has a lock to prevent concurrent fps calls.
    //  Note the lock does not protect the other frame listeners added by
    //  VideoSession::add_frame_listener().
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;
    //  Overwrites VideoSource::RenderedFrameListener::on_rendered_frame()
    //  This function is called when the video source finds a new rendered frame so
    //  VideoSession can update its internal fps record.
    //  This function is thread-safe as it has a lock to prevent concurrent fps calls.
    virtual void on_rendered_frame(WallClock timestamp) override;

    virtual void on_watchdog_timeout() override;


private:
    enum CommandType{
        RESET,
        SET_SOURCE,
        SET_RESOLUTION
    };
    struct Command{
        CommandType command_type;
        std::shared_ptr<VideoSourceDescriptor> device;
        Resolution resolution;
    };
    void run_commands();

    void internal_reset();
    void internal_set_source(
        const std::shared_ptr<VideoSourceDescriptor>& device,
        Resolution resolution = {}
    );
    void internal_set_resolution(Resolution resolution);


private:
    mutable std::recursive_mutex m_reset_lock;
    mutable SpinLock m_state_lock;

    Logger& m_logger;
    VideoSourceOption& m_option;

    mutable SpinLock m_fps_lock;
    EventRateTracker m_fps_tracker_source;
    EventRateTracker m_fps_tracker_rendered;

    std::shared_ptr<const VideoSourceDescriptor> m_descriptor;
    std::unique_ptr<VideoSource> m_video_source;

    //  We need to queue up all reset commands and run them on the main thread.
    //  This is needed to prevent re-entrant calls from event processing.
    SpinLock m_queue_lock;
    size_t m_recursion_depth = 0;
    std::deque<Command> m_queued_commands;

    ListenerSet<StateListener> m_state_listeners;
    ListenerSet<VideoFrameListener> m_frame_listeners;
};




}
#endif
