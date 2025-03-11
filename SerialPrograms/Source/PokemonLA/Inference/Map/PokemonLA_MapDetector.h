/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MapDetector_H
#define PokemonAutomation_PokemonLA_MapDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// Detect whether the map is opened
class MapDetector : public VisualInferenceCallback{
public:
    MapDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_bottom0;
    ImageFloatBox m_bottom1;
};


}
}
}
#endif
