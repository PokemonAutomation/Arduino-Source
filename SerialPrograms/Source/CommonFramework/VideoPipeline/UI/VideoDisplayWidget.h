/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWidget_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWidget_H

#include <deque>
#include <set>
#include "Common/Cpp/ImageResolution.h"
#include "VideoWidget.h"
#include "VideoOverlayWidget.h"


namespace PokemonAutomation{

//  The widget that owns the video window.
//  It consists of a VideoWidget that loads the video content from Switch and a VideoOverlayWidget
//  that renders inference boxes and other visualizations on top of the video content.
class VideoDisplayWidget : public QWidget{
public:
    VideoDisplayWidget(QWidget& parent, CameraSession& camera, VideoOverlaySession& overlay);

    operator bool(){ return m_video != nullptr; }
    VideoOverlayWidget& overlay(){ return *m_overlay; }

    void update_size(Resolution resolution = Resolution());

    // Move the video display widget to a new Qt window, so that we can make it full screen.
    // We need to go to a new window to do fullscreen because Qt cannot fullscreen a widget unless
    // it's a window.
    void move_to_new_window();

protected:
    // Override QWidget::mouseDoubleClickEvent().
    // When double click, call move_to_new_window() to move to a new window to be ready for full screen.
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    virtual void resizeEvent(QResizeEvent* event) override;

private:

    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



}
#endif
