/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <deque>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

class QImage;

namespace PokemonAutomation{

//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it will deadlock.
    //  Format must be "Format_ARGB32" or "Format_RGB32".
    virtual QImage snapshot() = 0;
};


class VideoOverlay{
public:
    //  Add/remove inference boxes.
    virtual void add_box(const ImageFloatBox& box, Color color) = 0;
    virtual void remove_box(const ImageFloatBox& box) = 0;
};


class InferenceBoxScope : public ImageFloatBox{
public:
    ~InferenceBoxScope(){
        m_overlay.remove_box(*this);
    }
    InferenceBoxScope(const InferenceBoxScope&) = delete;
    void operator=(const InferenceBoxScope&) = delete;


public:
    InferenceBoxScope(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        Color color = COLOR_RED
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
        Color color = COLOR_RED
    )
        : ImageFloatBox(p_x, p_y, p_width, p_height)
        , m_color(color)
        , m_overlay(overlay)
    {
        overlay.add_box(*this, color);
    }

private:
    Color m_color;
    VideoOverlay& m_overlay;
};


class VideoOverlaySet;




}
#endif
