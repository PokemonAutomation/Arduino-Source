/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoDisplayWidget_H
#define PokemonAutomation_VideoDisplayWidget_H

#include <deque>
#include <set>
#include "VideoWidget.h"
#include "VideoOverlayWidget.h"

namespace PokemonAutomation{


class VideoDisplayWidget : public QWidget, public VideoOverlay{
public:
    VideoDisplayWidget(QWidget& parent);

    operator bool(){ return m_video != nullptr; }
    VideoOverlayWidget& overlay(){ return *m_overlay; }

    void close_video();
    void set_video(VideoWidget* video); //  Takes ownership of video.

    QSize resolution() const;
    std::vector<QSize> resolutions() const;
    void set_resolution(const QSize& resolution);

    QImage snapshot();

private:
    virtual void add_box(const ImageFloatBox& box, QColor color) override;
    virtual void remove_box(const ImageFloatBox& box) override;

    void update_size();
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay = nullptr;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;
};



}
#endif
