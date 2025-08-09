/*  Box Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxShinyDetector_H
#define PokemonAutomation_PokemonSV_BoxShinyDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
//#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the shiny symbol on a pokemon in box system
class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_YELLOW, const ImageFloatBox& box = {0.878, 0.081, 0.028, 0.046});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

class BoxShinyWatcher : public DetectorToFinder<BoxShinyDetector>{
public:
    BoxShinyWatcher(
            Color color = COLOR_YELLOW,
            const ImageFloatBox& box = {0.878, 0.081, 0.028, 0.046},
            FinderType finder_type = FinderType::PRESENT
        )
            : DetectorToFinder("BoxShinyWatcher", finder_type, std::chrono::milliseconds(100), color, box)
    {}
};


}
}
}
#endif
