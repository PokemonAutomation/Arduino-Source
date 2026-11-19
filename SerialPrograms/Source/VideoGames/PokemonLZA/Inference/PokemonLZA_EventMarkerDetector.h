/*  Event Marker Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_EventMarkerDetector_H
#define PokemonAutomation_PokemonLZA_EventMarkerDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class EventMarkerDetector : public StaticScreenDetector{
public:
    EventMarkerDetector(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Valid only after a successful detection, excludes the flag to its right
    const ImageFloatBox& marker_box() const { return m_marker_box; }

private:
    Color m_color;
    ImageFloatBox m_search_box;
    ImageFloatBox m_marker_box;
};

class EventMarkerWatcher : public DetectorToFinder<EventMarkerDetector>{
public:
    EventMarkerWatcher(Color color = COLOR_YELLOW)
        : DetectorToFinder("EventMarkerWatcher", std::chrono::milliseconds(200), color)
    {}
};

// Require the marker to remain centered while its flag/flagless version settles
class CenteredEventMarkerWatcher : public DetectorToFinder<EventMarkerDetector>{
public:
    CenteredEventMarkerWatcher(Color color = COLOR_YELLOW)
        : DetectorToFinder("CenteredEventMarkerWatcher", std::chrono::milliseconds(400), color)
    {}

    virtual bool detect(const ImageViewRGB32& screen) override{
        if (!EventMarkerDetector::detect(screen)){
            return false;
        }
        const ImageFloatBox& box = marker_box();
        const double center = box.x + box.width / 2;
        return center >= 0.48 && center <= 0.52;
    }
};

}
}
}
#endif
