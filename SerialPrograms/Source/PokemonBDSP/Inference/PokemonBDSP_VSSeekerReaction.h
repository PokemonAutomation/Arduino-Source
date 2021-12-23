/*  VS Seeker Reaction Bubble
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BSSeekerReactionBubble_H
#define PokemonAutomation_PokemonBDSP_BSSeekerReactionBubble_H

#include <vector>
#include <deque>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


size_t find_seeker_bubbles(
    const QImage& image,
    std::vector<ImagePixelBox>& exclamation_marks
);



class VSSeekerReactionTracker : public VisualInferenceCallback{
public:
    VSSeekerReactionTracker(VideoOverlay& overlay, const ImageFloatBox& box);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

    const std::vector<ImagePixelBox>& reactions() const{ return m_bubbles; }

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    std::vector<ImagePixelBox> m_bubbles;
    std::deque<InferenceBoxScope> m_boxes;
};




}
}
}
#endif
