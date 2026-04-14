/*  Day Night State Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DayNightStateDetector_H
#define PokemonAutomation_PokemonLZA_DayNightStateDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


enum class DayNightState{
    DAY,
    NIGHT,
    UNKNOWN
};


// Detect current day/night state by analyzing the minimap
// Returns NIGHT if moon icon is detected at top of minimap
// Returns DAY if minimap is visible but no moon detected
// Returns UNKNOWN if unable to determine
class DayNightStateDetector : public StaticScreenDetector{
public:
    DayNightStateDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detector can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Get the detected day/night state from the last frame processed
    DayNightState get_state() const { return m_state; }

    void reset_state() override;

private:
    DayNightState m_state;
};


class DayNightStateWatcher : public DetectorToFinder<DayNightStateDetector>{
public:
    DayNightStateWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
         : DetectorToFinder("DayNightStateWatcher", hold_duration, color, overlay)
    {}

    //  Get the detected day/night state
    DayNightState get_state() const { return static_cast<const DayNightStateDetector*>(this)->get_state(); }
};



}
}
}
#endif
