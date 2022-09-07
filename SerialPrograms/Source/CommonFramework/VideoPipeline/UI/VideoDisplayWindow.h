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


//  A separate window to render video display.
//  This is used when the user wants to pop out the video display widget to a separate window.
//  A separate window has the benefit of becoming full screen.
//  This window is built by VideoDisplayWidget.
class VideoDisplayWindow : public QMainWindow{
public:
    VideoDisplayWindow(VideoDisplayWidget* display_widget);
    virtual ~VideoDisplayWindow() = default;

private:
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    VideoDisplayWidget* m_display_widget;
};



}
#endif
