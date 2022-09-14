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


//  Interface for forwarding keyboard commands from the VideoDisplayWidget to
//  whatever thing under it handles it.
struct CommandReceiver{
    //  Returns false if key is not handled. (pass it up to next handler)
    virtual bool key_press(QKeyEvent* event) = 0;
    virtual bool key_release(QKeyEvent* event) = 0;

    virtual void focus_in(QFocusEvent* event) = 0;
    virtual void focus_out(QFocusEvent* event) = 0;
};



//  The widget that owns the video window.
//  It consists of a VideoWidget that loads the video content from Switch and a VideoOverlayWidget
//  that renders inference boxes and other visualizations on top of the video content.
class VideoDisplayWidget : public QWidget{
public:
    VideoDisplayWidget(
        QWidget& parent, size_t id,
        CommandReceiver& command_receiver,
        CameraSession& camera,
        VideoOverlaySession& overlay
    );

    operator bool() const{ return m_video != nullptr; }
    size_t id() const{ return m_id; }

    VideoOverlayWidget& overlay(){ return *m_overlay; }
    CommandReceiver& command_receiver(){ return m_command_receiver; }
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
    const size_t m_id;
    CommandReceiver& m_command_receiver;
    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



}
#endif
