/*  Video Overlay Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoOverlaySet_H
#define PokemonAutomation_VideoOverlaySet_H

#include <deque>
#include "VideoFeed.h"

namespace PokemonAutomation{



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
