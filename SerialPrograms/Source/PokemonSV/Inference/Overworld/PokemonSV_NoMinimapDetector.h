/*  No Minimap Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_NoMinimapDetector_H
#define PokemonAutomation_PokemonSV_NoMinimapDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV_OverworldDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect that the minimap is not visible
class NoMinimapDetector : public StaticScreenDetector{
public:
    NoMinimapDetector(Logger& logger, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;


protected:
    const Color m_color;
    const ImageFloatBox m_ball;
    const OverworldDetector m_overworld;
};

class NoMinimapWatcher : public DetectorToFinder<NoMinimapDetector>{
public:
    NoMinimapWatcher(
        Logger& logger, 
        Color color = COLOR_RED, 
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(5000)
    )
        : DetectorToFinder("GradientArrowWatcher", hold_duration, logger, color)
    {}

};


}
}
}
#endif
