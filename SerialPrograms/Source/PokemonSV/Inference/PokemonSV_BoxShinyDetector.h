/*  Box Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxShinyDetector_H
#define PokemonAutomation_PokemonSV_BoxShinyDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the shiny symbol on a pokemon in box system
class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

class BoxShinyWatcher : public DetectorToFinder<BoxShinyDetector>{
public:
    BoxShinyWatcher(FinderType finder_type = FinderType::PRESENT, Color color = COLOR_YELLOW)
         : DetectorToFinder("BoxShinyWatcher", finder_type, std::chrono::milliseconds(100), color)
    {}
};


}
}
}
#endif
