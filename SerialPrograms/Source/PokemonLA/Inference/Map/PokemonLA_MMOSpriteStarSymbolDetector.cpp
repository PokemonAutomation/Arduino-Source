/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_MMOSpriteStarSymbolDetector.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



MMOSpriteStarSymbolDetector::MMOSpriteStarSymbolDetector(const ImageViewRGB32& frame, const std::vector<ImagePixelBox>& star_boxes)
    : VisualInferenceCallback("MMOSpriteStarSymbolDetector")
    , m_boxes(star_boxes), m_is_star(star_boxes.size(), false), m_rmsd(star_boxes.size(), 0.0)
    , m_symbol_colors(star_boxes.size(), FloatPixel())
    , m_frame_width(frame.width()), m_frame_height(frame.height())
{
    for(size_t i = 0; i < m_boxes.size(); i++){
        ImageViewRGB32 ref = extract_box_reference(frame, m_boxes[i]);
        m_initial_images.push_back(std::move(ref));
    }
}

void MMOSpriteStarSymbolDetector::make_overlays(VideoOverlaySet& items) const{
    for(const auto& box : m_boxes){
        items.add(COLOR_RED, pixelbox_to_floatbox(m_frame_width, m_frame_height, box));
    }
}

//  Return true if the inference session should stop.
bool MMOSpriteStarSymbolDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    for(size_t i = 0; i < m_boxes.size(); i++){
        ImageViewRGB32 ref = extract_box_reference(frame, m_boxes[i]);
        double rmsd = ImageMatch::pixel_RMSD(m_initial_images[i], ref);
        m_rmsd[i] += rmsd;

        m_symbol_colors[i] += frame.pixel(m_boxes[i].center_x(), m_boxes[i].center_y());
    }

    m_num_frames++;

    if (m_num_frames < 30){
        return false;
    }

    for(size_t i = 0; i < m_boxes.size(); i++){
        double rmsd = m_rmsd[i] / m_num_frames;
        if (rmsd <= 20){
            continue;
        }

        auto symbol_color = m_symbol_colors[i] / m_num_frames;
        if (symbol_color.r <= symbol_color.g * 1.3){
            // It's not very red, so it's not a berry symbol
            m_is_star[i] = true;
        }
    }

    // QImage output = frame;
    // for(size_t i = 0; i < m_boxes.size(); i++){
    //     draw_box(output, m_boxes[i], combine_rgb(255, 0,0));
    //     draw_box(output, ImagePixelBox(m_boxes[i].center_x()-1, m_boxes[i].center_y()-1, m_boxes[i].center_x()+1, m_boxes[i].center_y()+1), 
    //         combine_rgb(0, 255,0));
    // }
    // output.save("test_MMO_star.png");

    return true;
}



}
}
}
