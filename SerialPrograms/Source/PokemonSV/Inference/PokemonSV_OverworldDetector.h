/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_OverworldDetector_H
#define PokemonAutomation_PokemonSV_OverworldDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class OverworldDetector : public StaticScreenDetector{
public:
    OverworldDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool detect_ball(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_ball;
    ImageFloatBox m_radar;
};
class OverworldWatcher : public DetectorToFinder<OverworldDetector>{
public:
    OverworldWatcher(Color color = COLOR_RED)
         : DetectorToFinder("OverworldWatcher", std::chrono::milliseconds(1000), color)
    {}
};



}
}
}
#endif
