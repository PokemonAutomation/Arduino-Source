/* Direction Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DirectionArrowDetector_H
#define PokemonAutomation_PokemonLZA_DirectionArrowDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect the direction arrow on the map that shows where the device is facing
class DirectionArrowDetector : public StaticScreenDetector{
public:
    DirectionArrowDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Get the last detected angle in degrees [0, 360)
    // Returns -1 if no arrow was detected
    double detected_angle_deg() const { return m_detected_angle; }

private:
    Color m_color;
    ImageFloatBox m_search_box;

    // Detection results
    double m_detected_angle;  // in degrees [0, 360), or -1 if not detected
};

class DirectionArrowWatcher : public DetectorToFinder<DirectionArrowDetector>{
public:
    DirectionArrowWatcher(
        Color color = COLOR_RED,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
         : DetectorToFinder("DirectionArrowWatcher", hold_duration, color)
    {}
};




}
}
}
#endif
