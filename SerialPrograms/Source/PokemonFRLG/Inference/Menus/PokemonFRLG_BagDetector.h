/*  Bag Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BagDetector_H
#define PokemonAutomation_PokemonFRLG_BagDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

class BagDetector : public StaticScreenDetector{
public:
    BagDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_bag_shadow_box;
    ImageFloatBox m_pocket_underline_box;
    ImageFloatBox m_menu_top_box;
    ImageFloatBox m_menu_right_box;
};
class BagWatcher : public DetectorToFinder<BagDetector>{
public:
    BagWatcher(Color color)
        : DetectorToFinder("BagWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
