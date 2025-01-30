/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWidget_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWidget_H

//#include "Common/Cpp/ValueDebouncer.h"
#include "Common/Qt/WidgetStackFixedAspectRatio.h"
#include "VideoWidget.h"
#include "VideoOverlayWidget.h"

class QLineEdit;

namespace PokemonAutomation{

class VideoDisplayWindow;


//  Interface for forwarding keyboard commands from the VideoDisplayWidget to
//  whatever thing under it handles it.
struct CommandReceiver{
    virtual void key_press(QKeyEvent* event) = 0;
    virtual void key_release(QKeyEvent* event) = 0;

    virtual void focus_in(QFocusEvent* event) = 0;
    virtual void focus_out(QFocusEvent* event) = 0;
};





class VideoDisplayWidget;

class VideoSourceFPS : public OverlayStat{
public:
    VideoSourceFPS(VideoDisplayWidget& parent)
        : m_parent(parent)
    {}
    virtual OverlayStatSnapshot get_current() override;

private:
    VideoDisplayWidget& m_parent;
};

class VideoDisplayFPS : public OverlayStat{
public:
    VideoDisplayFPS(VideoDisplayWidget& parent)
        : m_parent(parent)
    {}
    virtual OverlayStatSnapshot get_current() override;

private:
    VideoDisplayWidget& m_parent;
};



//  The widget that owns the video window.
//  It consists of a VideoWidget that loads the video content from Switch and a VideoOverlayWidget
//  that renders inference boxes and other visualizations on top of the video content.
class VideoDisplayWidget : public WidgetStackFixedAspectRatio{
public:
    VideoDisplayWidget(
        QWidget& parent, QLayout& holder,
        size_t id,
        CommandReceiver& command_receiver,
        CameraSession& camera,
        VideoOverlaySession& overlay
    );
    ~VideoDisplayWidget();

    operator bool() const{ return m_video != nullptr; }
    size_t id() const{ return m_id; }

    VideoOverlayWidget& overlay(){ return *m_overlay; }
    CommandReceiver& command_receiver(){ return m_command_receiver; }

    // Move the video display widget to a new Qt window, so that we can make it full screen.
    // We need to go to a new window to do fullscreen because Qt cannot fullscreen a widget unless
    // it's a window.
    void move_to_new_window();
    void move_back_from_window();

protected:
    // Override QWidget::mouseDoubleClickEvent().
    // When double click, call move_to_new_window() to move to a new window to be ready for full screen.
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    friend class VideoSourceFPS;
    friend class VideoDisplayFPS;

    QLayout& m_holder;
    const size_t m_id;
    CommandReceiver& m_command_receiver;
    VideoOverlaySession& m_overlay_session;

    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    QWidget* m_underlay = nullptr;
    QLineEdit* m_width_box = nullptr;
    QLineEdit* m_height_box = nullptr;
    int m_last_width = 0;
    int m_last_height = 0;

    std::unique_ptr<VideoDisplayWindow> m_window;

    VideoSourceFPS m_source_fps;
    VideoDisplayFPS m_display_fps;
};



}
#endif
