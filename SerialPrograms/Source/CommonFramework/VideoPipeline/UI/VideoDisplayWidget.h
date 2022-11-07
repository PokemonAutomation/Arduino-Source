/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWidget_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWidget_H

#include <deque>
#include <set>
#include "VideoWidget.h"
#include "VideoOverlayWidget.h"


namespace PokemonAutomation{

class VideoDisplayWindow;


//  Interface for forwarding keyboard commands from the VideoDisplayWidget to
//  whatever thing under it handles it.
struct CommandReceiver{
    //  Returns false if key is not handled. (pass it up to next handler)
    virtual bool key_press(QKeyEvent* event) = 0;
    virtual bool key_release(QKeyEvent* event) = 0;

    virtual void focus_in(QFocusEvent* event) = 0;
    virtual void focus_out(QFocusEvent* event) = 0;
};



class WidgetStackFixedAspectRatio : public QWidget{
public:
    enum SizePolicy{
        EXPAND_TO_BOX,
        ADJUST_HEIGHT_TO_WIDTH,
    };

public:
    WidgetStackFixedAspectRatio(QWidget& parent, SizePolicy size_policy, double aspect_ratio = 16 / 9.);

    void set_size_policy(SizePolicy size_policy);
    void set_aspect_ratio(double aspect_ratio);
    void add_widget(QWidget& widget);

    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void clear_fixed_dimensions();
    void resize_to_box(QSize enclosing_box);
    void resize_to_width(int width);

    void update_size(QSize size);

private:
    SizePolicy m_size_policy;
    double m_aspect_ratio;
    std::set<QWidget*> m_widgets;

    QWidget* m_detached_internal;
    QWidget* m_stack_holder;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



//  The widget that owns the video window.
//  It consists of a VideoWidget that loads the video content from Switch and a VideoOverlayWidget
//  that renders inference boxes and other visualizations on top of the video content.
class VideoDisplayWidget : public WidgetStackFixedAspectRatio, public OverlayStat{
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

private:
    virtual OverlayStatSnapshot get_current() const override;

private:
    QLayout& m_holder;
    const size_t m_id;
    CommandReceiver& m_command_receiver;
    VideoOverlaySession& m_overlay_session;
    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;
    std::unique_ptr<VideoDisplayWindow> m_window;
};



}
#endif
