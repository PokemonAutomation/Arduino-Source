/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlayWidget_H
#define PokemonAutomation_VideoOverlayWidget_H

#include <map>
#include <QWidget>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{


class VideoOverlayWidget : public QWidget{
public:
    VideoOverlayWidget(QWidget& parent);

    //  Add/remove inference boxes.
    void add_box(const ImageFloatBox& box, Color color);
    void remove_box(const ImageFloatBox& box);

    void update_size(const QSize& widget_size, const QSize& video_size);

private:
    void paintEvent(QPaintEvent*) override;

private:
    QSize m_video_size;
    QSize m_display_size;
    int m_offset_x;
    double m_scale;

    SpinLock m_lock;
    std::map<const ImageFloatBox*, Color> m_boxes;
};


}
#endif

