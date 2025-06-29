/*  Frozen Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect if the entire screen is frozen.
 */

#ifndef PokemonAutomation_CommonTools_FrozenImageDetector_H
#define PokemonAutomation_CommonTools_FrozenImageDetector_H

#include "Common/Cpp/Color.h"
//#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{


class FrozenImageDetector : public VisualInferenceCallback{
public:
    FrozenImageDetector(std::chrono::milliseconds timeout, double rmsd_threshold);
    FrozenImageDetector(
        Color color, const ImageFloatBox& box,
        std::chrono::milliseconds timeout, double rmsd_threshold
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Color m_color;
    ImageFloatBox m_box;
    std::chrono::milliseconds m_timeout;
    double m_rmsd_threshold;
    VideoSnapshot m_previous;
};



}
#endif
