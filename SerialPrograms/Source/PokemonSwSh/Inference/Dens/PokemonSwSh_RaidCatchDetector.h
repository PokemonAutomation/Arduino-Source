/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidCatchDetector_H
#define PokemonAutomation_PokemonSwSh_RaidCatchDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidCatchDetector : public VisualInferenceCallback{
public:
    RaidCatchDetector(VideoOverlay& overlay);

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_left0;
    ImageFloatBox m_right0;
//    ImageFloatBox m_left1;
//    ImageFloatBox m_right1;
    ImageFloatBox m_text0;
    ImageFloatBox m_text1;
    SelectionArrowFinder m_arrow;

    size_t m_trigger_count = 0;
};


}
}
}
#endif

