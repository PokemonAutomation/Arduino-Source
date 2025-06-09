/*  Picnic Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PicnicDetector_H
#define PokemonAutomation_PokemonSV_PicnicDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the player character is in a picnic
class PicnicDetector : public StaticScreenDetector{
public:
    PicnicDetector(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    WhiteButtonDetector m_button_b;
    WhiteButtonDetector m_button_minus;
    WhiteButtonDetector m_button_y;
};

class PicnicWatcher : public DetectorToFinder<PicnicDetector>{
public:
    PicnicWatcher(Color color = COLOR_YELLOW)
         : DetectorToFinder("PicnicWatcher", std::chrono::milliseconds(1000), color)
    {}
};



}
}
}
#endif
