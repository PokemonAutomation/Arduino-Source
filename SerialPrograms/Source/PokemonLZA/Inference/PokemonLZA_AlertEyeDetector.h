/*  Alert Eye Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_AlertEyeDetector_H
#define PokemonAutomation_PokemonLZA_AlertEyeDetector_H

#include <optional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect the red warning symbol when you are under attack by wild pokemon
class AlertEyeDetector : public StaticScreenDetector{
public:
    AlertEyeDetector(
        Color color,
        VideoOverlay* overlay
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class AlertEyeWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_alert_eye_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class AlertEyeWatcher : public DetectorToFinder<AlertEyeDetector>{
public:
    AlertEyeWatcher(
        Color color,
        VideoOverlay* overlay,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("AlertEyeWatcher", hold_duration, color, overlay)
    {}
};




}
}
}
#endif
