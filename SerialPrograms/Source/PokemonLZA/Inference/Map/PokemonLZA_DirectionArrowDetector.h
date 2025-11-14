/* Direction Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DirectionArrowDetector_H
#define PokemonAutomation_PokemonLZA_DirectionArrowDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect the direction arrow on the map that shows where the device is facing
class DirectionArrowDetector : public StaticScreenDetector{
public:
    DirectionArrowDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    // TODO: Add member variables for implementation
};

class DirectionArrowWatcher : public DetectorToFinder<DirectionArrowDetector>{
public:
    DirectionArrowWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("DirectionArrowWatcher", hold_duration, color, overlay)
    {}
};




}
}
}
#endif
