/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FishingDetector_H
#define PokemonAutomation_PokemonSwSh_FishingDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonSwSh{


class FishingMissDetector : public VisualInferenceCallback{
public:
    FishingMissDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_hook_box;
    ImageFloatBox m_miss_box;
};

class FishingHookDetector : public VisualInferenceCallback{
public:
    FishingHookDetector(VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    VideoOverlay& m_overlay;
    ImageFloatBox m_hook_box;
    std::deque<OverlayBoxScope> m_marks;
};



}
}
}
#endif
