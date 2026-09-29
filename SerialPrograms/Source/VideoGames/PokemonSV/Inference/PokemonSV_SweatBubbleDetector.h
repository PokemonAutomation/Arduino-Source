/*  Sweat Bubble Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SweatBubbleDetector_H
#define PokemonAutomation_PokemonSV_SweatBubbleDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class SweatBubbleDetector : public StaticScreenDetector{
public:
    SweatBubbleDetector(Color color, const ImageFloatBox& box = {0.11, 0.81, 0.06, 0.10});

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_box;
};
class SweatBubbleWatcher : public DetectorToFinder<SweatBubbleDetector>{
public:
    SweatBubbleWatcher(Color color = COLOR_RED)
         : DetectorToFinder("SweatBubbleWatcher", std::chrono::milliseconds(100), color)
    {}
};




}
}
}
#endif
