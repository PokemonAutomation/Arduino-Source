/*  Video Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSource_H
#define PokemonAutomation_VideoPipeline_VideoSource_H

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
        m_source_frame_listeners.add(listener);
    }
    void remove_source_frame_listener(VideoFrameListener& listener){
        m_source_frame_listeners.remove(listener);
    }
    void add_rendered_frame_listener(RenderedFrameListener& listener){
        m_rendered_frame_listeners.add(listener);
    }
    void remove_rendered_frame_listener(RenderedFrameListener& listener){
        m_rendered_frame_listeners.remove(listener);
    }


public:
    VideoSource()
        : m_sanitizer("VideoSource")
    {}
    virtual ~VideoSource() = default;

    virtual Resolution current_resolution() const = 0;
    virtual std::vector<Resolution> supported_resolutions() const = 0;

    virtual VideoSnapshot snapshot() = 0;


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
    virtual QWidget* make_QtWidget(QWidget* parent) = 0;


private:
    ListenerSet<VideoFrameListener> m_source_frame_listeners;
    ListenerSet<RenderedFrameListener> m_rendered_frame_listeners;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
