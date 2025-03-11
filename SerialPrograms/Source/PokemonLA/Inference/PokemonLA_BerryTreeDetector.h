/*  Berry Tree Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_BerryTreeDetector_H
#define PokemonAutomation_PokemonLA_BerryTreeDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BerryTreeDetector : public VisualInferenceCallback{
public:
    BerryTreeDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
};


}
}
}
#endif
