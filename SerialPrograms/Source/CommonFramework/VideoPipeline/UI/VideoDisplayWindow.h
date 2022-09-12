/*  Video Display Window
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoDisplayWindow_H
#define PokemonAutomation_VideoPipeline_VideoDisplayWindow_H

#include <QMainWindow>

namespace PokemonAutomation{

class VideoDisplayWidget;

namespace NintendoSwitch{
    class SwitchSystemWidget;
}


//  A separate window to render video display.
//  This is used when the user wants to pop out the video display widget to a separate window.
//  A separate window has the benefit of becoming full screen.
//  This window is built by VideoDisplayWidget.
class VideoDisplayWindow : public QMainWindow{
public:
    // The constructor of VideoDisplayWindow transfers of the ownership of VideoDisplayWidget from its parent
    // SwitchSystemWidget to VideoDisplayWindow, and making SwitchSystemWidget the parent of the
    // VideoDisplayWindow.
    // The constructor then displays the window.
    VideoDisplayWindow(VideoDisplayWidget* display_widget);
    virtual ~VideoDisplayWindow() = default;

private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    // Override key and focus event handlers to pass those events back to the parent widget.
    // The parent widget is SwitchSystemWidget. It needs to listen to key and focus events to
    // realize virtual keyboard functionality.
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

    VideoDisplayWidget* m_display_widget;

    NintendoSwitch::SwitchSystemWidget* m_parent_switch_system_widget;

    bool m_full_screen = false;
};



}
#endif
