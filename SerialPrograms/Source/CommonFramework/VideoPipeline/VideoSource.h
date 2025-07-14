/*  Video Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSource_H
#define PokemonAutomation_VideoPipeline_VideoSource_H

#include <vector>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/ImageResolution.h"
#include "Common/Cpp/ListenerSet.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

class QWidget;

namespace PokemonAutomation{




class VideoSource{
public:
    struct RenderedFrameListener{
        virtual void on_rendered_frame(WallClock timestamp) = 0;
    };

    void add_source_frame_listener(VideoFrameListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        m_source_frame_listeners.add(listener);
    }
    void remove_source_frame_listener(VideoFrameListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        m_source_frame_listeners.remove(listener);
    }
    void add_rendered_frame_listener(RenderedFrameListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        m_rendered_frame_listeners.add(listener);
    }
    void remove_rendered_frame_listener(RenderedFrameListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        m_rendered_frame_listeners.remove(listener);
    }


public:
    VideoSource(bool allow_watchdog_reset)
        : m_allow_watchdog_reset(allow_watchdog_reset)
        , m_sanitizer("VideoSource")
    {}
    virtual ~VideoSource() = default;


public:
    bool allow_watchdog_reset() const{
        return m_allow_watchdog_reset;
    }

    virtual Resolution current_resolution() const = 0;
    virtual const std::vector<Resolution>& supported_resolutions() const = 0;

    virtual VideoSnapshot snapshot_latest_blocking() = 0;
    virtual VideoSnapshot snapshot_recent_nonblocking() = 0;


protected:
    void report_source_frame(std::shared_ptr<const VideoFrame> frame){
        auto scope_check = m_sanitizer.check_scope();
        m_source_frame_listeners.run_method_unique(&VideoFrameListener::on_frame, frame);
    }
    void report_rendered_frame(WallClock timestamp){
        auto scope_check = m_sanitizer.check_scope();
        m_rendered_frame_listeners.run_method_unique(&RenderedFrameListener::on_rendered_frame, timestamp);
    }


public:
    virtual QWidget* make_display_QtWidget(QWidget* parent) = 0;


private:
    ListenerSet<VideoFrameListener> m_source_frame_listeners;
    ListenerSet<RenderedFrameListener> m_rendered_frame_listeners;

    const bool m_allow_watchdog_reset;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
