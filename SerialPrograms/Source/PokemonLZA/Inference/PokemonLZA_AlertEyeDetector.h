/*  Alert Eye Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_AlertEyeDetector_H
#define PokemonAutomation_PokemonLZA_AlertEyeDetector_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect the red warning symbol when you are under attack by wild pokemon
class AlertEyeDetector : public StaticScreenDetector{
public:
    AlertEyeDetector(Color color, VideoOverlay* overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class AlertEyeWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_alert_eye_box;
    const ImageFloatBox m_initial_alert_eye_box;

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

class AlertEyeTracker final : public AlertEyeDetector, public VisualInferenceCallback{
public:
    AlertEyeTracker(Color color, VideoOverlay* overlay, WallDuration min_duration);

    bool currently_active() const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    mutable SpinLock m_lock;
    WallDuration m_min_duration;
    WallClock m_first_detection;
};




}
}
}
#endif
