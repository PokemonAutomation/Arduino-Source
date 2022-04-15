/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWidget_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWidget_H

#include <functional>
#include <deque>
#include <set>
#include "VideoWidget.h"
#include "VideoOverlayWidget.h"

namespace PokemonAutomation{

//  The widget that owns the video window.
//  It is made by a VideoWidget that loads the video content from Switch
//  and a VideoOverlayWidget that renders inference boxes on top of the video
//  content to visualize inferences when running programs.
//  The VideoWidget is assigned by calling VideoDisplayWidget::set_video().
//  This function is called by CameraSelectorWidget.
class VideoDisplayWidget : public QWidget, public VideoOverlay{
public:
    VideoDisplayWidget(QWidget& parent);


    operator bool(){ return m_video != nullptr; }
    VideoOverlayWidget& overlay(){ return *m_overlay; }

    void close_video();

    //  Set video using an already constructed video. This takes ownership of the video.
    void set_video(VideoWidget* video);

    //  Set video using a factory lambda. The video is constructed using this class as the parent.
    void set_video(std::function<VideoWidget*(QWidget& parent)> video_factory);

    QSize resolution() const;
    std::vector<QSize> resolutions() const;
    void set_resolution(const QSize& resolution);

    QImage snapshot(WallClock* timestamp = nullptr);

private:
    virtual void add_box(const ImageFloatBox& box, Color color) override;
    virtual void remove_box(const ImageFloatBox& box) override;

    void update_size();
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



}
#endif
