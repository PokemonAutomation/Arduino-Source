/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlay_H
#define PokemonAutomation_VideoOverlay_H

#include <set>
#include <QWidget>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{


class VideoOverlay : public QWidget{
public:
    VideoOverlay(QWidget& parent);

    //  Add/remove inference boxes.
    void operator+=(const InferenceBox& box);
    void operator-=(const InferenceBox& box);

    void update_size(const QSize& widget_size, const QSize& video_size);

private:
    void paintEvent(QPaintEvent*) override;

private:
    QSize m_video_size;
    QSize m_display_size;
    int m_offset_x;
    double m_scale;

    SpinLock m_lock;
    std::set<const InferenceBox*> m_boxes;
};


}
#endif

