/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <QImage>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{


class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it will deadlock.
    virtual QImage snapshot() = 0;
};


class VideoOverlay{
public:
    //  Add/remove inference boxes.
    virtual void add_box(const ImageFloatBox& box, QColor color) = 0;
    virtual void remove_box(const ImageFloatBox& box) = 0;
};


class InferenceBoxScope : public ImageFloatBox{
public:
    InferenceBoxScope(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        QColor color = Qt::red
    )
        : ImageFloatBox(box)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }
    InferenceBoxScope(
        VideoOverlay& overlay,
        double p_x, double p_y,
        double p_width, double p_height,
        QColor color = Qt::red
    )
        : ImageFloatBox(p_x, p_y, p_width, p_height)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }
#if 1
    InferenceBoxScope(
        VideoOverlay& overlay,
        QColor color,
        double p_x, double p_y,
        double p_width, double p_height
    ) = delete;
    InferenceBoxScope(
        VideoOverlay& overlay,
        double p_x, double p_y,
        double p_width, double p_height,
        Qt::GlobalColor color
    )
        : ImageFloatBox(p_x, p_y, p_width, p_height)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }
#endif
    ~InferenceBoxScope(){
        m_overlay.remove_box(*this);
    }

private:
    QColor m_color;
    VideoOverlay& m_overlay;
};


}
#endif
