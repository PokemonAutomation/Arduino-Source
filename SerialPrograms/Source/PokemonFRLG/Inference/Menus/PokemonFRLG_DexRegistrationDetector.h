/*  Dex Registration Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_DexRegistrationDetector_H
#define PokemonAutomation_PokemonFRLG_DexRegistrationDetector_H

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

class DexRegistrationDetector : public StaticScreenDetector{
public:
    DexRegistrationDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top_box;
    ImageFloatBox m_header_box;
    ImageFloatBox m_divider_box;
    ImageFloatBox m_body_box;
    ImageFloatBox m_bottom_box;
};
class DexRegistrationWatcher : public DetectorToFinder<DexRegistrationDetector>{
public:
    DexRegistrationWatcher(Color color)
        : DetectorToFinder("DexRegistrationWatcher", std::chrono::milliseconds(50), color)
    {}
};



}
}
}

#endif
