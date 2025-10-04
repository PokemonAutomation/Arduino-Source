/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidCatchDetector_H
#define PokemonAutomation_PokemonSwSh_RaidCatchDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidCatchDetector : public StaticScreenDetector{
public:
    RaidCatchDetector(VideoOverlay& overlay);

    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual void make_overlays(VideoOverlaySet& items) const override;


private:
    ImageFloatBox m_left0;
    ImageFloatBox m_right0;
//    ImageFloatBox m_left1;
//    ImageFloatBox m_right1;
    ImageFloatBox m_text0;
    ImageFloatBox m_text1;
    SelectionArrowFinder m_arrow;
};
class RaidCatchWatcher : public DetectorToFinder<RaidCatchDetector>{
public:
    RaidCatchWatcher(VideoOverlay& overlay)
        : DetectorToFinder("RaidCatchWatcher", std::chrono::milliseconds(500), overlay)
    {}
};


}
}
}
#endif

