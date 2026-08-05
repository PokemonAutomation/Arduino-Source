/*  Video Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  VideoDisplayWidget renders a window for video stream from a Switch console or 
 *  other sources. Stream control UI is not part of this class.
 * 
 *  The actual QWidget that displays the video content is not created by this class.
 *  But rather, this class receives the content widget from
 *  VideoSession.current_source()->make_display_QtWidget().
 * 
 *  VideoDisplayWidget is repsonsible for handling aspect ratio and passing key events
 *  around. It also uses VideoDisplayWindow to pop up the stream view into a standalone
 *  QMainWindow.
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWidget_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWidget_H

//#include "Common/Cpp/ValueDebouncer.h"
#include "Common/Qt/WidgetStackFixedAspectRatio.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "VideoOverlayWidget.h"

class QLineEdit;

namespace PokemonAutomation{

class VideoDisplayWindow;


//  Interface for forwarding keyboard and focus events from the VideoDisplayWidget to
//  whatever thing inheriting CommandReceiver.
struct CommandReceiver{
    virtual void key_press(QKeyEvent* event) = 0;
    virtual void key_release(QKeyEvent* event) = 0;

    virtual void focus_in(QFocusEvent* event) = 0;
    virtual void focus_out(QFocusEvent* event) = 0;
};





class VideoDisplayWidget;

// Render the FPS of the incoming video stream as a text overlay on the video window
class VideoSourceFPS : public OverlayStat{
public:
    VideoSourceFPS(VideoDisplayWidget& parent)
        : m_parent(parent)
    {}
    virtual OverlayStatSnapshot get_current() override;

private:
    VideoDisplayWidget& m_parent;
};

// Render the FPS of the rendering thread of the video window as a text overlay on the window
class VideoDisplayFPS : public OverlayStat{
public:
    VideoDisplayFPS(VideoDisplayWidget& parent)
        : m_parent(parent)
    {}
    virtual OverlayStatSnapshot get_current() override;

private:
    VideoDisplayWidget& m_parent;
};



//  The widget that owns the video stream window.
//  It consists of a VideoWidget that loads the video content from Switch or other sources
//  and a VideoOverlayWidget that renders text overlays, inference boxes and other visualizations
//  on top of the video content.
class VideoDisplayWidget : public WidgetStackFixedAspectRatio, private VideoSession::StateListener{
public:
    // holder: parent's layout that holds this VideoDisplayWidget
    // id: the ID to differentiate multiple video views. For multi-Switch automation programs, this id
    //   is the console ID.
    // command_receiver: whenever there is a keyboard or focus event, forward the event to this
    //   command_receiver object.
    VideoDisplayWidget(
        QWidget& parent, QLayout& holder,
        size_t id,
        CommandReceiver& command_receiver,
        VideoSession& video_session,
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
    virtual void post_startup(VideoSource* source) override;
    virtual void pre_shutdown() override;

    // Override QWidget::mouseDoubleClickEvent().
    // When double click, call move_to_new_window() to move to a new window to be ready for full screen.
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void clear_video_source();

private:
    friend class VideoSourceFPS;
    friend class VideoDisplayFPS;

    QLayout& m_holder;
    const size_t m_id;
    CommandReceiver& m_command_receiver;
    VideoSession& m_video_session;
    VideoOverlaySession& m_overlay_session;

    QWidget* m_video = nullptr;
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
