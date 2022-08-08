/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlayWidget_H
#define PokemonAutomation_VideoPipeline_VideoOverlayWidget_H

#include <map>
#include <QWidget>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"

namespace PokemonAutomation{


class VideoOverlayWidget : public QWidget, private VideoOverlaySession::Listener{
public:
    ~VideoOverlayWidget();
    VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session);

    //  Add/remove inference boxes.
    void add_box(const ImageFloatBox& box, Color color);
    void remove_box(const ImageFloatBox& box);

    void update_size(const QSize& widget_size, const QSize& video_size);

private:
    virtual void box_update(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& boxes) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    VideoOverlaySession& m_session;

    QSize m_video_size;
    QSize m_display_size;
    int m_offset_x;
    double m_scale;

    SpinLock m_lock;
    std::shared_ptr<const std::vector<VideoOverlaySession::Box>> m_boxes;
};


}
#endif

