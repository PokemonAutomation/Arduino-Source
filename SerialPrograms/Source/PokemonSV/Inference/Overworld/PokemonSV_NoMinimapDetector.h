/*  No Minimap Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_NoMinimapDetector_H
#define PokemonAutomation_PokemonSV_NoMinimapDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV_OverworldDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect that the minimap is not visible
class NoMinimapDetector : public StaticScreenDetector{
public:
    NoMinimapDetector(Logger& logger, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;


protected:
    const Color m_color;
    const ImageFloatBox m_ball;
    OverworldDetector m_overworld;
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
