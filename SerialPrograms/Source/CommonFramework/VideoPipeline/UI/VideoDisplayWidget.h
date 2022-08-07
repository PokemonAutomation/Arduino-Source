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
#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
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
    VideoDisplayWidget(QWidget& parent, CameraSession& session);

    operator bool(){ return m_video != nullptr; }
    VideoOverlayWidget& overlay(){ return *m_overlay; }

    void update_size(Resolution resolution = Resolution());

private:
    virtual void add_box(const ImageFloatBox& box, Color color) override;
    virtual void remove_box(const ImageFloatBox& box) override;

    virtual void resizeEvent(QResizeEvent* event) override;

private:
    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



}
#endif
