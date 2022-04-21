/*  Video Overlay Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlaySet_H
#define PokemonAutomation_VideoOverlaySet_H

#include <deque>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{



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



class VideoOverlaySet{
public:
    VideoOverlaySet(VideoOverlay& overlay)
        : m_overlay(overlay)
    {}

    void clear(){
        m_boxes.clear();
    }
    void add(Color color, const ImageFloatBox& box){
        m_boxes.emplace_back(m_overlay, box, color);
    }

private:
    VideoOverlay& m_overlay;
    std::deque<InferenceBoxScope> m_boxes;
};



}
#endif
