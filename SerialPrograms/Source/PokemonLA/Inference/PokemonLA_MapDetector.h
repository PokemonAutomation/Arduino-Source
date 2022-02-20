/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_MapDetector_H
#define PokemonAutomation_PokemonLA_MapDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MapDetector : public VisualInferenceCallback{
public:
    MapDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    ImageFloatBox m_bottom0;
    ImageFloatBox m_bottom1;
};


}
}
}
#endif
