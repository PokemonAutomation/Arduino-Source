/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLA_MMOSpriteStarSymbolDetector.h"


//#include <iostream>
//using std::cout;
//using std::endl;

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
    // If we already collect enough frames, then we should stop doing detection
    if (m_num_frames >= 30){
        return true;
    }

    // Collect image color stats:
    for(size_t i = 0; i < m_boxes.size(); i++){
        ImageViewRGB32 ref = extract_box_reference(frame, m_boxes[i]);
        double rmsd = ImageMatch::pixel_RMSD(m_initial_images[i], ref);
        m_rmsd[i] += rmsd;

        m_symbol_colors[i] += frame.pixel(m_boxes[i].center_x(), m_boxes[i].center_y());
    }

    m_num_frames++;

    // We don't have enough frames, continue collection:
    if (m_num_frames < 30){
        return false;
    }

    // We have enough frames, compute stats to determine whether each sprite has a star symbol.
    // Caller of this detector can query `m_is_star` to know the detection result.
    // Details of the detection can be queried via `m_rmsd` and `m_symbol_colors`
    // 
    for(size_t i = 0; i < m_boxes.size(); i++){
        // Compute average values:
        m_rmsd[i] = m_rmsd[i] / (double)m_num_frames;
        m_symbol_colors[i] = m_symbol_colors[i] / (double)m_num_frames;

        if (m_rmsd[i] <= 20){
            continue;
        }

        if (m_symbol_colors[i].r <= m_symbol_colors[i].g * 1.2 && m_symbol_colors[i].g > 200 && m_symbol_colors[i].r > 220){
            // It's not very red, so it's not a berry symbol
            m_is_star[i] = true;
        }
    }

    // ImageRGB32 output = frame.copy();
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
