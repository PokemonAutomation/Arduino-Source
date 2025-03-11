/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MarkFinder_H
#define PokemonAutomation_PokemonBDSP_MarkFinder_H

#include <vector>
#include <deque>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonBDSP{


std::vector<ImagePixelBox> find_exclamation_marks(const ImageViewRGB32& image);



class MarkTracker : public VisualInferenceCallback{
public:
    MarkTracker(VideoOverlay& overlay, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    std::deque<OverlayBoxScope> m_marks;
};

class MarkDetector : public MarkTracker{
public:
    using MarkTracker::MarkTracker;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};



}
}
}
#endif
