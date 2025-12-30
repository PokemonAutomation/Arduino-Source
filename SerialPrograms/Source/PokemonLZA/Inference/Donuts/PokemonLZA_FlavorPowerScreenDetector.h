/*  Flavor Power Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_FlavorPowerScreenDetector_H
#define PokemonAutomation_PokemonLZA_FlavorPowerScreenDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detector for the flavor power screen showing donut effects
class FlavorPowerScreenDetector : public StaticScreenDetector{
public:
    FlavorPowerScreenDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const ImageFloatBox m_left_white_area;
    const ImageFloatBox m_bottom_white_area;
    const ImageFloatBox m_donut_area;
};

class FlavorPowerScreenWatcher : public DetectorToFinder<FlavorPowerScreenDetector>{
public:
    FlavorPowerScreenWatcher(
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
         : DetectorToFinder("FlavorPowerScreenWatcher", hold_duration)
    {}
};


}
}
}
#endif
