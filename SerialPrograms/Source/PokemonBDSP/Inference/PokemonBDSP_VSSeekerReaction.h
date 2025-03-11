/*  VS Seeker Reaction Bubble
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BSSeekerReactionBubble_H
#define PokemonAutomation_PokemonBDSP_BSSeekerReactionBubble_H

#include <vector>
#include <deque>
#include <QSize>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::vector<ImagePixelBox> find_seeker_bubbles(const ImageViewRGB32& image);



class VSSeekerReactionTracker : public VisualInferenceCallback{
public:
    VSSeekerReactionTracker(VideoOverlay& overlay, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    QSize dimensions() const{ return m_dimensions; }
    const std::vector<ImagePixelBox>& reactions() const{ return m_bubbles; }

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    QSize m_dimensions;
    std::vector<ImagePixelBox> m_bubbles;
    std::deque<OverlayBoxScope> m_boxes;
};




}
}
}
#endif
